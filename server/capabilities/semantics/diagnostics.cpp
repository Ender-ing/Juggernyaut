/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#include "diagnostics.hpp"

// Diagnostics
#include "../../../core/diagnostics/Diagnostic.hpp"
#include "../../../core/diagnostics/DiagnosticsManager.hpp"

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
        lsp::MessageType internal_getMessageType(const lsp::Opt<lsp::DiagnosticSeverity> &severity) {
            if (!severity.has_value()) {
                return lsp::MessageType::Info;
            }
            switch (severity.value()) {
            case lsp::DiagnosticSeverity::Error:
                return lsp::MessageType::Error;
            case lsp::DiagnosticSeverity::Warning:
                return lsp::MessageType::Warning;
            case lsp::DiagnosticSeverity::Information:
                return lsp::MessageType::Info;
            case lsp::DiagnosticSeverity::Hint:
            default:
                return lsp::MessageType::Log;
            }
        }

        lsp::ServerEndpoint *io = nullptr;
        std::unique_ptr<Diagnostics::DiagnosticsManager<lsp::Diagnostic>> diagManager = nullptr;
        void setupGlobalDiagnostics(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver) {
            // Data::Host::VFS *vfs = driver.host.vfs;

            // Update diagnostics I/O endpoint
            io = &endpoint;

            diagManager = std::make_unique<Diagnostics::DiagnosticsManager<lsp::Diagnostic>>(
                driver.configs.hooks,
                [](const Diagnostics::Diagnostic &diag) -> lsp::Diagnostic {
                    return internal_diagToLSP(diag);
                },
                Diagnostics::DiagnosticsManager<lsp::Diagnostic>::ManagerCallbacks{
                    .onFileFlush = [](const std::string &uri, const std::vector<lsp::Diagnostic> &diags) {
                        if (io ==  nullptr) {
                            return;
                        }

                        io->textDocumentPublishDiagnostics({
                            .uri = lsp::DocumentUri::fileUriFromPath(uri),
                            .diagnostics = diags
                        });
                    },
                    .onComponentFlush = [](const lsp::Diagnostic &diag) {
                        if (io ==  nullptr) {
                            return;
                        }

                        const lsp::MessageType msgType = internal_getMessageType(diag.severity);
                        const std::string &msg = std::get<std::string>(diag.message);

                        // Show a popup notification in the editor for Errors/Warnings,
                        // and log quietly to the Output panel for Info/Hints:
                        if (msgType == lsp::MessageType::Error || msgType == lsp::MessageType::Warning) {
                            io->windowShowMessage({
                                .type = msgType,
                                .message = msg
                            });
                        } else {
                            io->windowLogMessage({
                                .type = msgType,
                                .message = msg
                            });
                        }
                    }
                }
            );
        }
    }
}
