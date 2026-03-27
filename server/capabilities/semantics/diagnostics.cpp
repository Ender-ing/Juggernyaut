/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#include "diagnostics.hpp"

// Diagnostics
#include "../../../core/diagnostics/Diagnostic.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

namespace Capabilities {
    namespace Semantics {
        // ANTLR4 functions
        void SyntaxErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol,
            size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e) {

            Diagnostics::Diagnostic diag = Diagnostics::antlrToDiagnostic(recognizer, offendingSymbol, line,
                charPositionInLine, msg, e);

            lsp::Diagnostic error;

            // Get the starting position
            error.range.start.line = std::move(diag.range.start.line);
            error.range.start.character = std::move(diag.range.start.character);

            // Determine the end position
            error.range.end.line = std::move(diag.range.end.line);
            error.range.end.character = std::move(diag.range.end.character);

            switch (diag.severity) {
            case Diagnostics::Severity::Error:
                error.severity = lsp::DiagnosticSeverity::Error;
                break;
            case Diagnostics::Severity::Warning:
                error.severity = lsp::DiagnosticSeverity::Warning;
                break;
            case Diagnostics::Severity::Info:
                error.severity = lsp::DiagnosticSeverity::Information;
                break;
            case Diagnostics::Severity::Hint:
                error.severity = lsp::DiagnosticSeverity::Hint;
                break;
            default:
                error.severity = lsp::DiagnosticSeverity::MAX_VALUE;
                break;
            }

            //error.message = (std::string) this->stage + " Error: " + msg + "\n(Token Text: '" + tokenText + "')";
            error.message = std::move(diag.message);
            error.code = std::move(diag.code);
            error.source = "Jug";
            this->diags.push_back(error);
        }
        void SyntaxErrorListener::reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs) {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Ambiguity reported from index ",
            //     startIndex ," to index " , stopIndex, Console::END_REPORT);
        }
        void SyntaxErrorListener::reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
            antlr4::atn::ATNConfigSet *configs) {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Attempting full context reported from index ",
            //     startIndex ," to index " , stopIndex, Console::END_REPORT);
        }
        void SyntaxErrorListener::reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, size_t prediction,
            antlr4::atn::ATNConfigSet *configs) {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Context sensitivity reported from index ",
            //         startIndex ," to index " , stopIndex, Console::END_REPORT);
        }

        void validateDocumentSyntax(lsp::MessageHandler &messageHandler, Store::Document &doc) {
            // TMP
            std::vector<lsp::Diagnostic> diagnostics;

            SyntaxErrorListener lexerDebugErrorListener("Lexer", diagnostics);
            SyntaxErrorListener parserDebugErrorListener("Parser", diagnostics);

            // REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Tokens: \n");
            Parser::Debug::syntaxCheck(doc.getRawContent(),
                [](const std::string tokenText){
                    // REPORT(tokenText, "\n");
                },
                [](const std::string treeText){
                    // REPORT(Console::END_REPORT);
                    // REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Parse Tree: \n", treeText, Console::END_REPORT);
                }, &lexerDebugErrorListener, &parserDebugErrorListener);

            // Publish the diagnostics to the editor
            auto diagParams = lsp::notifications::TextDocument_PublishDiagnostics::Params{};
            diagParams.uri = lsp::DocumentUri::fromPath(doc.uri); // The URI of the file you just checked
            diagParams.diagnostics = diagnostics;
            messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(diagParams));
        }
    }
}
