/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#include "diagnostics.hpp"

// Diagnostics
#include "../../../core/diagnostics/Diagnostic.hpp"
#include "../../../core/diagnostics/antlr.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

namespace Capabilities {
    namespace Semantics {
        // Internal
        lsp::DiagnosticSeverity internal_getSeverity(Diagnostics::Severity &severity) {
            switch (severity) {
            case Diagnostics::Severity::Error:
                return lsp::DiagnosticSeverity::Error;
            case Diagnostics::Severity::Warning:
                return lsp::DiagnosticSeverity::Warning;
            case Diagnostics::Severity::Info:
                return lsp::DiagnosticSeverity::Information;
            case Diagnostics::Severity::Hint:
                return lsp::DiagnosticSeverity::Hint;
            default:
                return lsp::DiagnosticSeverity::MAX_VALUE;
            }
        }

        // ANTLR4 functions
        void SyntaxListener::onSyntaxError(Diagnostics::Diagnostic diag) {

            lsp::Diagnostic diagnostic;

            // Get the starting position
            diagnostic.range.start.line = std::move(diag.range.start.line);
            diagnostic.range.start.character = std::move(diag.range.start.character);

            // Determine the end position
            diagnostic.range.end.line = std::move(diag.range.end.line);
            diagnostic.range.end.character = std::move(diag.range.end.character);

            diagnostic.severity = internal_getSeverity(diag.severity);

            diagnostic.message = std::move(diag.message);
            diagnostic.code = std::move(diag.code);
            diagnostic.source = "Jug";
            this->diags.push_back(diagnostic);
        }
        void SyntaxListener::onAmbiguity(Diagnostics::Diagnostic diag) {

            lsp::Diagnostic diagnostic;

            // Get the starting position
            diagnostic.range.start.line = std::move(diag.range.start.line);
            diagnostic.range.start.character = std::move(diag.range.start.character);

            // Determine the end position
            diagnostic.range.end.line = std::move(diag.range.end.line);
            diagnostic.range.end.character = std::move(diag.range.end.character);

            diagnostic.severity = internal_getSeverity(diag.severity);

            diagnostic.message = std::move(diag.message);
            diagnostic.code = std::move(diag.code);
            diagnostic.source = "Jug";
            this->diags.push_back(diagnostic);
        }

        void validateDocumentSyntax(lsp::MessageHandler &messageHandler, Store::Document &doc) {
            // TMP
            std::vector<lsp::Diagnostic> diagnostics;

            SyntaxListener lexerDebugErrorListener(diagnostics);
            SyntaxListener parserDebugErrorListener(diagnostics);

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
