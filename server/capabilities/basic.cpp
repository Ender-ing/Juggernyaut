/**
 * @brief
 * Handle basic capabilities
**/

// Common headers
#include "basic.hpp"

#include "../base/info.hpp"

namespace Capabilities {
    lsp::MessageHandler *handler = nullptr;
    void configureProtocol(lsp::MessageHandler &messageHandler, bool &received_shutdown, Store::DocumentStore &store) {
        handler = &messageHandler;
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
            [&received_shutdown]() {
                printMessage<lsp::notifications::Exit>();
                exit(received_shutdown ? 0 : 1);
            }
        ).add<lsp::notifications::TextDocument_DidOpen>(
            [&store](lsp::notifications::TextDocument_DidOpen::Params&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                const std::string sourceCode = params.textDocument.text;

                // Load doc
                store.initDocument(rawUri);
                const Store::Document *docPtr = store.getDocument(rawUri);
                Store::Document doc = *docPtr;
                if (docPtr == nullptr) {
                    // THROW AN ERROR!
                    return;
                }

                doc.setIsInEditor(true);
                doc.setRawContent(sourceCode);
            }
        )
        // Listen for when a file is modified
        .add<lsp::notifications::TextDocument_DidChange>(
            [&store](lsp::notifications::TextDocument_DidChange::Params&& params) {
                // Note: If you requested Full sync in your InitializeResult, 
                // params.contentChanges[0].text will contain the entire updated file.
                if (!params.contentChanges.empty()) {
                    const std::string rawUri = std::string(params.textDocument.uri.path());
                    const std::string updatedSourceCode = std::visit(
                        [](const auto& changeEvent) { return changeEvent.text; }, 
                        params.contentChanges[0]
                    );

                    // Load doc
                    store.initDocument(rawUri);
                    const Store::Document *docPtr = store.getDocument(rawUri);
                    Store::Document doc = *docPtr;
                    if (docPtr == nullptr) {
                        // THROW AN ERROR!
                        return;
                    }

                    doc.setIsInEditor(true);
                    doc.setRawContent(updatedSourceCode);
                }
            }
        );
    }
}
