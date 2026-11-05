/**
 * @brief
 * Parser implementations
**/

#include "Parser.hpp"

// ANTLR4 runtime strategies & prediction modes
#include "atn/ParserATNSimulator.h"
#include "atn/PredictionMode.h"
#include "BailErrorStrategy.h"
#include "DefaultErrorStrategy.h"
#include "Exceptions.h"

// Internal
#include "internal/JugLexer.hpp"
#include "internal/JugParser.hpp"
#include "internal/cleanup.hpp"

// Listeners & Visitors
#include "listeners/ANTLRDiagnosticListener.hpp"
#include "visitors/ASTGenVisitor.hpp"

namespace Parser {
    void Parser::emitDiagnostic(const File *file, Diagnostics::Diagnostic &diag) const {
        diag.file = file;
        if (this->hooks.onDiagnostic != nullptr) {
            this->hooks.onDiagnostic(diag);
        }
    }
    void Parser::reportAntlrData(const File *file, std::string &&data) const {
        if (this->hooks.onDiagnostic == nullptr) {
            return;
        }

        Diagnostics::Diagnostic diag = {.file = file};
        diag.severity = Diagnostics::Severity::Info;
        diag.code = 200011;
        diag.message = std::move(data);

        this->hooks.onDiagnostic(diag);
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

        Listeners::ANTLRDiagnosticListener diagListener(
            file,
            this->hooks.onDiagnostic,
            this->env->debug.reportAmbiguity
        );

        // [STAGE 1] ANTLR4 Lexer
        antlr4::ANTLRInputStream input(file->content);
        Internal::JugLexer lexer(&input);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&diagListener);

        antlr4::CommonTokenStream tokens(&lexer);

        tokens.fill();
        if (this->env->debug.reportANTLRData) {
            std::string tokenStreamStr;
            for (const antlr4::Token *token : tokens.getTokens()) {
                if (!tokenStreamStr.empty()) {
                    tokenStreamStr += '\n';
                }
                tokenStreamStr += token->toString();
            }
            this->reportAntlrData(file, std::move(tokenStreamStr));
        }

        if (this->hooks.onLexerContextEnd != nullptr) {
            this->hooks.onLexerContextEnd(file);
        }

        // Respect early termination
        if (this->configs.terminateAfterLexer) {
            result.hasSyntaxErrors = diagListener.hasErrors();

            if (this->hooks.onContextEnd != nullptr) {
                this->hooks.onContextEnd(file);
            }
            return result;
        }

        // [STAGE 2] Parser (Fast SLL -> Slow LL Fallback)
        Internal::JugParser parser(&tokens);
        parser.removeErrorListeners();

        auto *interpreter = parser.getInterpreter<antlr4::atn::ParserATNSimulator>();
        antlr4::tree::ParseTree *tree = nullptr;

        // If ambiguity reporting is explicitly requested, go straight to LL
        if (!(this->env->debug.reportAmbiguity) && interpreter != nullptr) {
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
        }

        if (this->env->debug.reportANTLRData && tree != nullptr) {
            this->reportAntlrData(file, tree->toStringTree(&parser));
        }

        // [STAGE 3] AST & Import Extraction
        if (tree != nullptr) {
            Visitors::ASTGenVisitor visitor(file, this->hooks.onDiagnostic, result);
            visitor.visit(tree);
        }

        result.hasSyntaxErrors = diagListener.hasErrors();

        if (this->hooks.onParserContextEnd != nullptr) {
            this->hooks.onParserContextEnd(file);
        }

        // TMP (Move into the session manager)
        if (this->hooks.onContextEnd != nullptr) {
            this->hooks.onContextEnd(file);
        }

        return result;
    }

    void Parser::cleanup() {
        Internal::flushAntlrCache(); // ...maybe this isn't worth it?
    }
}
