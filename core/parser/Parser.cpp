/**
 * @brief
 * Parser implementations
**/

#include "Parser.hpp"

// C++ headers
#include <stdexcept>

// tree-sitter
#include "tree-sitter.hpp"

// Listeners & Visitors
#include "visitors/CSTVisitor.hpp"

// Grab tree sitter language
extern "C" const TSLanguage* tree_sitter_jug(void); 

namespace Parser {
    // Handle tree sitter parseer
    namespace Internal {
        thread_local auto tsParser = ts::Parser::create(ts::Language{tree_sitter_jug()});
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

        if (!Internal::tsParser.has_value()) {
            // This shouldn't happen...
            throw std::runtime_error("TSParser generation failed! (Please make sure that the imported TS language and the internal TS library use the same version of TS!)");
        }

        ts::Parser& parser = *Internal::tsParser;
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
        auto newTree = std::make_shared<ts::Tree>(std::move(*treeRes));

        // Report (debug) data (if needed)
        const bool reportCSTData = (this->env != nullptr) && this->env->debug.reportCSTData;
        if (reportCSTData) {
            std::string sExpr = newTree->getRootNode().getSExpr();
            this->sendCSTDataReport(file, std::move(sExpr));
        }
        return newTree;
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
        if (file == nullptr) {
            return result;
        }

        // Notify context start
        if (this->hooks.onContextStart != nullptr) {
            this->hooks.onContextStart(file);
        }

        // Generate/Update CST
        InternalCSTHandle cst = this->updateCST(file);

        // TMP
        auto tree = std::static_pointer_cast<ts::Tree>(cst);
        Visitors::CSTVisitor visitor(file, this->hooks.onDiagnostic);
        if (tree != nullptr) {
            visitor.visit(*tree);
        }
        result.hasSyntaxErrors = visitor.hasSyntaxErrors();

        // ...

        // Handle post CST-logic
        this->negotiateCSTCache(file, cst);

        // [STAGE 3] AST & Import Extraction
        /*if (tree != nullptr) {
            Visitors::ASTGenVisitor visitor(file, this->hooks.onDiagnostic, result);
            visitor.visit(tree);
        }*/

        result.hasSyntaxErrors = false;//diagListener.hasErrors();

        // TMP
        Data::Diagnostics::Diagnostic diag = {.message = "Meow!", .severity = Data::Diagnostics::Severity::Warning};
        this->emitDiagnostic(file, diag);

        if (this->hooks.onParserContextEnd != nullptr) {
            this->hooks.onParserContextEnd(file);
        }

        return result;
    }

    void Parser::cleanup() {
        // Internal::flushAntlrCache(); // ...maybe this isn't worth it?
    }
}
