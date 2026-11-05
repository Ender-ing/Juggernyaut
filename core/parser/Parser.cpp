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
            this->setup->debug.reportAmbiguity
        );

        // [STAGE 1] ANTLR4 Lexer
        antlr4::ANTLRInputStream input(file->content);
        Internal::JugLexer lexer(&input);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&diagListener);

        antlr4::CommonTokenStream tokens(&lexer);

        if (!(this->setup->debug.reportANTLRData) || this->hooks.onANTLRTokenDetected == nullptr) {
            tokens.fill();
        } else {
            // Indicate the start of the stream
            this->hooks.onANTLRTokenDetected(1, "");

            antlr4::Token *token = nullptr;
            do {
                token = tokens.LT(1);
                this->hooks.onANTLRTokenDetected(2, token->toString());
                if (token->getType() != antlr4::Token::EOF) {
                    tokens.consume();
                }
            } while (token->getType() != antlr4::Token::EOF);
            tokens.seek(0);

            // Indicate the end of the stream
            this->hooks.onANTLRTokenDetected(3, "");
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
        if (!(this->setup->debug.reportAmbiguity) && interpreter != nullptr) {
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

        if (this->setup->debug.reportANTLRData && this->hooks.onANTLRTreeGenerated != nullptr && tree != nullptr) {
            this->hooks.onANTLRTreeGenerated(tree->toStringTree(&parser));
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

        // ...

        if (this->hooks.onContextEnd != nullptr) {
            this->hooks.onContextEnd(file);
        }

        return result;
    }

    void Parser::cleanup() {
        Internal::flushAntlrCache(); // ...maybe this isn't worth it?
    }
}
