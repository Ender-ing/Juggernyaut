/**
 * @brief
 * Handle basic capabilities
**/

// Common headers
#include "basic.hpp"

#include "../base/info.hpp"

// Session
#include "../core/session/session.hpp"

// Store
#include "../store/DocumentStore.hpp"

namespace Capabilities {
    void configureProtocol(lsp::MessageHandler &messageHandler, Session::Session &session, int &exit_code) {
        bool received_shutdown = false;
        Store::DocumentStore *store = static_cast<Store::DocumentStore*>(session.store);
        messageHandler.add<lsp::requests::Initialize>(
            [](lsp::requests::Initialize::Params&& params) {
                printMessage<lsp::requests::Initialize>(params);

                /*
                 * Respond with an InitializeResult containing some basic server info and capabilities
                 */

                return lsp::requests::Initialize::Result {
                    .capabilities = {
                        .positionEncoding = lsp::PositionEncodingKind::UTF16,
                        .textDocumentSync = lsp::TextDocumentSyncOptions {
                            .openClose = true,
                            .change = lsp::TextDocumentSyncKind::Full,
                            .save = true
                        },
                        .hoverProvider = false,
                    },
                    .serverInfo = lsp::InitializeResultServerInfo {
                        .name = "Juggernyaut Language Server",
                        .version = Base::Info::version
                    },
                };
            }
        ).add<lsp::requests::Shutdown>(
            [&received_shutdown]() {
                printMessage<lsp::requests::Shutdown>();
                received_shutdown = true;
                return lsp::requests::Shutdown::Result();
            }
        ).add<lsp::notifications::Exit>(
            [&received_shutdown, &exit_code]() {
                printMessage<lsp::notifications::Exit>();
                exit_code = received_shutdown ? 0 : 1;
            }
        ).add<lsp::notifications::TextDocument_DidOpen>(
            [&store, &session](lsp::notifications::TextDocument_DidOpen::Params&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                std::string sourceCode = std::move(params.textDocument.text);

                // Load doc
                store->syncRaw(rawUri, sourceCode);
                store->syncStatus(rawUri, true);

                // Refresh the session
                Session::initiate(session);
            }
        ).add<lsp::notifications::TextDocument_DidChange>(
            [&store, &session](lsp::notifications::TextDocument_DidChange::Params&& params) {
                // Note: If you requested Full sync in your InitializeResult, 
                // params.contentChanges[0].text will contain the entire updated file.
                if (!params.contentChanges.empty()) {
                    const std::string rawUri = std::string(params.textDocument.uri.path());
                    std::string updatedSourceCode = std::visit(
                        [](auto& changeEvent) {
                            return std::move(changeEvent.text);
                        },
                        params.contentChanges[0]
                    );

                    // Load doc
                    store->syncRaw(rawUri, updatedSourceCode);
                    store->syncStatus(rawUri, true);

                    // Refresh the session
                    Session::initiate(session);
                }
            }
        ).add<lsp::notifications::TextDocument_DidClose>(
            [&store, &session](lsp::notifications::TextDocument_DidClose::Params&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                // Load doc
                store->syncStatus(rawUri, false);


                // Refresh the session
                Session::initiate(session);
            }
        );
    }
}
