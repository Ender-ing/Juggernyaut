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
        diag.code = 200011;
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
        const std::string source = file->content.toString();

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

        file->cst = std::make_shared<ts::Tree>(std::move(*treeRes));
        file->cstVersion = file->version;
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

        const bool reportAmbiguity = (this->env != nullptr) && this->env->debug.reportAmbiguity;
        const bool reportCSTData = (this->env != nullptr) && this->env->debug.reportCSTData;

        (void)reportAmbiguity;

        // Generate/Update CST
        this->updateCST(file);

        /*Listeners::ANTLRDiagnosticListener diagListener(
            file,
            this->hooks.onDiagnostic,
            reportAmbiguity
        );*/

        // [STAGE 1] ANTLR4 Lexer
        /*antlr4::ANTLRInputStream input(file->content.getChunk(0));
        Internal::JugLexer lexer(&input);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&diagListener);

        antlr4::CommonTokenStream tokens(&lexer);

        tokens.fill();*/
        if (reportCSTData) {
            /*std::string tokenStreamStr;
            for (const antlr4::Token *token : tokens.getTokens()) {
                if (!tokenStreamStr.empty()) {
                    tokenStreamStr += '\n';
                }
                tokenStreamStr += token->toString();
            }*/
            //this->sendCSTDataReport(file, std::move(tokenStreamStr));
        }

        // [STAGE 2] Parser (Fast SLL -> Slow LL Fallback)
        /*Internal::JugParser parser(&tokens);
        parser.removeErrorListeners();

        auto *interpreter = parser.getInterpreter<antlr4::atn::ParserATNSimulator>();
        antlr4::tree::ParseTree *tree = nullptr;

        // If ambiguity reporting is explicitly requested, go straight to LL
        if (!reportAmbiguity && interpreter != nullptr) {
            interpreter->setPredictionMode(antlr4::atn::PredictionMode::SLL);
            parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());

            try {
                tree = parser.prog(); // Root
            } catch (const antlr4::ParseCancellationException &) {
                // SLL failed: rewind and fall back to full LL parsing with diagnostics
                tokens.seek(0);
                parser.reset();
                parser.addErrorListener(&diagListener);
                parser.setErrorHandler(std::make_shared<antlr4::DefaultErrorStrategy>());
                interpreter->setPredictionMode(antlr4::atn::PredictionMode::LL);

                tree = parser.prog(); // Root
            }
        } else {
            if (interpreter != nullptr) {
                interpreter->setPredictionMode(antlr4::atn::PredictionMode::LL);
            }
            parser.addErrorListener(&diagListener);

            tree = parser.prog(); // Root
        }*/

        if (reportCSTData) { //&& tree != nullptr) {
            //this->sendCSTDataReport(file, tree->toStringTree(&parser));
        }

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
