/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#include "diagnostics.hpp"

// Diagnostics
#include "../../../core/diagnostics/Diagnostic.hpp"
#include "../../../core/diagnostics/antlr.hpp"

namespace Capabilities {
    namespace Semantics {
        // LSP functions
        lsp::DiagnosticSeverity internal_getSeverity(const Diagnostics::Severity &severity) {
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
        lsp::Diagnostic internal_diagToLSP(const Diagnostics::Diagnostic &diag) {

            lsp::Diagnostic diagnostic;

            // Get the starting position
            diagnostic.range.start.line = diag.range.start.line;
            diagnostic.range.start.character = diag.range.start.character;

            // Determine the end position
            diagnostic.range.end.line = diag.range.end.line;
            diagnostic.range.end.character = diag.range.end.character;

            diagnostic.severity = internal_getSeverity(diag.severity);

            diagnostic.message = diag.message;
            diagnostic.code = diag.code;
            diagnostic.source = "Jug";

            return diagnostic;
        }

        void setupParserDiagnostics(lsp::MessageHandler &messageHandler, Session::Session &session) {
            Data::Store::SourceStore *store = session.store;

            session.hooks.parser.onContextEnd = [store, &messageHandler](const Data::Store::SourceId srcId) {
                std::unique_ptr<Data::Store::Source> &source = store->getSourceById(srcId);

                std::vector<lsp::Diagnostic> diagnostics;

                source->visitParserDiagnostics([&diagnostics](const Diagnostics::Diagnostic &diag) {
                    diagnostics.push_back(std::move(internal_diagToLSP(diag)));
                });

                // Publish the diagnostics to the editor
                auto diagParams = lsp::notifications::TextDocument_PublishDiagnostics::Params{};
                diagParams.uri = lsp::DocumentUri::fromPath(source->uri); // The URI of the file you just checked
                diagParams.diagnostics = std::move(diagnostics);
                messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(diagParams));
            };
        }
    }
}
