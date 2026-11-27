/**
 * @brief
 * Parser implementations
**/

#include "Parser.hpp"

// C++ headers
#include <stdexcept>

// mimalloc
#ifdef JUG_USE_MIMALLOC
#include <mimalloc.h>
#endif

// tree-sitter
#include "tree-sitter.hpp"

// Visitors, lowering & rules
#include "lowering/ASTBuilder.hpp"
#include "rules/JugASTMapper.hpp"
#include "visitors/CSTVisitor.hpp"

// Grab tree sitter language
extern "C" const TSLanguage* tree_sitter_jug(void); 

namespace Parser {
    // Handle tree sitter parseer
    namespace Internal {
        static auto& getTSParser() {
            // Force tree-sitter to use mimalloc
            #ifdef JUG_USE_MIMALLOC
            static const bool allocatorInit = []() {
                ts_set_allocator(mi_malloc, mi_calloc, mi_realloc, mi_free);
                return true;
            }();
            (void)allocatorInit;
            #endif

            thread_local auto tsParser = ts::Parser::create(ts::Language{tree_sitter_jug()});
            return tsParser;
        }
        static Visitors::CSTVisitor& getCSTVisitor() {
            thread_local Visitors::CSTVisitor visitor;
            return visitor;
        }
        static Lowering::ASTBuilder& getASTBuilder() {
            thread_local Lowering::ASTBuilder builder(&Rules::getJugLoweringTable(ts::Language{tree_sitter_jug()}));
            return builder;
        }
    }

    void Parser::emitDiagnostic(const File *file, Data::Diagnostics::Diagnostic &diag) const {
        diag.file = file;
        if (this->hooks.onDiagnostic != nullptr) {
            this->hooks.onDiagnostic(diag);
        }
    }
    void Parser::sendCSTDataReport(const File *file, std::string &&data) const {
        if (this->hooks.onDiagnostic == nullptr) {
            return;
        }

        Data::Diagnostics::Diagnostic diag = {.file = file};
        diag.severity = Data::Diagnostics::Severity::Info;
        diag.code = 200002;
        diag.message = std::move(data);

        this->hooks.onDiagnostic(diag);
    }

    // tree-sitter
    InternalCSTHandle Parser::updateCST(const File *file) const {
        if (file == nullptr || !file->isValid) {
            return nullptr;
        }
        // Check shared cache (if enabled for this session)
        Data::Parser::CSTEntry cached{};
        if (this->cstCache != nullptr) {
            cached = this->cstCache->checkout(file->id, file->version);
            if (cached.tree != nullptr && cached.version == file->version) {
                return cached.tree;
            }
        }

        // Grab the thread-local tree-sitter parser
        auto& tsParser = Internal::getTSParser();
        if (!tsParser.has_value()) {
            // This shouldn't happen...
            throw std::runtime_error("TSParser generation failed! (Please make sure that the imported TS language and the internal TS library use the same version of TS!)");
        }
        ts::Parser& parser = *tsParser;

        const std::string_view source = file->content.view();
        std::expected<ts::Tree, ts::Error> treeRes =
            std::unexpected{ts::Error{.kind = ts::ErrorKind::ParseFailed}};
        const bool canReuseTree =
            cached.tree != nullptr &&
            cached.version < file->version &&
            cached.version == file->deltasBaseVersion &&
            !(file->pendingDeltas.empty());
        if (canReuseTree) {
            // Move out of `cached.tree` so `use_count()` accurately reflects external readers
            auto oldTree = std::static_pointer_cast<ts::Tree>(std::move(cached.tree));
            if (oldTree.use_count() > 1) {
                // Another reader still holds this snapshot: O(1) shallow copy before in-place edit
                oldTree = std::make_shared<ts::Tree>(ts_tree_copy(oldTree->raw()));
            }
            for (const auto& delta : file->pendingDeltas) {
                oldTree->edit(ts::InputEdit{
                    .start  = {delta.startByte,  {delta.startPoint.line,  delta.startPoint.column}},
                    .oldEnd = {delta.oldEndByte, {delta.oldEndPoint.line, delta.oldEndPoint.column}},
                    .newEnd = {delta.newEndByte, {delta.newEndPoint.line, delta.newEndPoint.column}},
                });
            }
            treeRes = parser.parse(source, *oldTree);
        } else {
            treeRes = parser.parse(source);
        }

        if (!treeRes.has_value()) {
            // Parse failed (ErrorKind::ParseFailed)
            SET_CODE(diag, Error, 200001);
            this->emitDiagnostic(file, diag);
            return nullptr;
        }
        return std::make_shared<ts::Tree>(std::move(*treeRes));
    }
    constexpr size_t MAX_CACHED_CST_BYTES = 4 * 1024 * 1024; // 4 MB (~120,000 LOC)
    constexpr uint32_t MAX_CACHED_ROOT_NODES = 50000;
    void Parser::negotiateCSTCache(const File *file, const InternalCSTHandle &cst) const {
        if (this->cstCache == nullptr || file == nullptr) {
            return;
        }
        if (cst == nullptr) {
            this->cstCache->evict(file->id);
            return;
        }
        auto tree = std::static_pointer_cast<ts::Tree>(cst);

        // Filter out costly CSTs!
        if (file->content.byteLength() > MAX_CACHED_CST_BYTES ||
            tree->getRootNode().getNumChildren() > MAX_CACHED_ROOT_NODES) {
            this->cstCache->evict(file->id);
            return;
        }

        // Cache the CST!
        this->cstCache->set(file->id, cst, file->version);
    }

    ParseResult Parser::parse(const File *file) const {
        ParseResult result;
        if (file == nullptr || this->env == nullptr) {
            return result;
        }

        // Notify context start
        if (this->hooks.onContextStart != nullptr) {
            this->hooks.onContextStart(file);
        }

        // Generate/Update CST
        InternalCSTHandle cst = this->updateCST(file);
        
        // Borrow raw pointer — avoids atomic shared_ptr refcount increment/decrement
        if (const auto *tree = static_cast<const ts::Tree*>(cst.get()); tree != nullptr) {
                // Report (debug) data
                if (this->env->debug.reportCSTData) {
                    this->sendCSTDataReport(file, tree->getRootNode().getSExpr());
                }

            Visitors::CSTVisitor &visitor = Internal::getCSTVisitor();
            Lowering::ASTBuilder &builder = Internal::getASTBuilder();

            builder.reset(file, &result.imports);
            visitor.configure(
                Visitors::CSTVisitorConfigs{
                    .reportUnusedCSTNodes = this->env->debug.reportUnusedCSTNodes,
                },
                file,
                &this->hooks.onDiagnostic
            );
            visitor.visit(*tree, builder);

            result.hasSyntaxErrors = visitor.hasSyntaxErrors();
            result.ast = builder.takeAST();
        }

        // Handle CST cache
        this->negotiateCSTCache(file, cst);

        // TMP
        if (this->env->debug.sayMeow) {
            Data::Diagnostics::Diagnostic diag = {.message = "Meow ;3", .severity = Data::Diagnostics::Severity::Warning};
            this->emitDiagnostic(file, diag);
        }

        if (this->hooks.onParserContextEnd != nullptr) {
            this->hooks.onParserContextEnd(file);
        }

        return result;
    }

    void Parser::cleanup() {
        // Internal::flushAntlrCache(); // ...maybe this isn't worth it?
    }
}
