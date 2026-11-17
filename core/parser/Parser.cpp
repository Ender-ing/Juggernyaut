/**
 * @brief
 * Parser implementations
**/

#include "Parser.hpp"

// tree-sitter
#include "tree-sitter.hpp"

// Listeners & Visitors
// ...

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
        diag.code = 200003;
        diag.message = std::move(data);

        this->hooks.onDiagnostic(diag);
    }

    // tree-sitter
    void Parser::updateCST(const File *file) const {
        if (file == nullptr || !file->isValid) {
            return;
        }
        if (file->cst != nullptr && file->cstVersion == file->version) {
            return;
        }
        if (!Internal::tsParser.has_value()) {
            // Creation failed (ErrorKind::LanguageIncompatible)
            SET_CODE(diag, Error, 200001);
            this->emitDiagnostic(file, diag);
            return;
        }

        ts::Parser& parser = *Internal::tsParser;
        const std::string_view source = file->content.view();

        std::expected<ts::Tree, ts::Error> treeRes =
            std::unexpected{ts::Error{.kind = ts::ErrorKind::ParseFailed}};

        if (file->cst != nullptr && !file->pendingDeltas.empty()) {
            auto oldTree = std::static_pointer_cast<ts::Tree>(file->cst);
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
            SET_CODE(diag, Error, 200002);
            this->emitDiagnostic(file, diag);
            return;
        }

        // Update file's CST content
        file->cst = std::make_shared<ts::Tree>(std::move(*treeRes));
        file->cstVersion = file->version;

        // Report (debug) data (if needed)
        const bool reportCSTData = (this->env != nullptr) && this->env->debug.reportCSTData;
        if (reportCSTData) {
            auto tree = std::static_pointer_cast<ts::Tree>(file->cst);
            std::string sexp = tree->getRootNode().getSExpr();
            this->sendCSTDataReport(file, std::move(sexp));
        }
    }
    constexpr size_t MAX_CACHED_CST_BYTES = 4 * 1024 * 1024; // 4 MB (~120,000 LOC)
    constexpr uint32_t MAX_CACHED_ROOT_NODES = 50000;
    void Parser::negotiateCSTCache(const File *file) const {
        if (file->cst == nullptr) {
            return;
        }
        auto tree = std::static_pointer_cast<ts::Tree>(file->cst);
        if (!(this->configs.allowCSTCaching) || file->content.byteLength() > MAX_CACHED_CST_BYTES ||
            tree->getRootNode().getNumChildren() > MAX_CACHED_ROOT_NODES) {
            file->cst.reset();
            file->cstVersion = 0;
        }
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
        this->updateCST(file);
        // ...
        this->negotiateCSTCache(file);

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
