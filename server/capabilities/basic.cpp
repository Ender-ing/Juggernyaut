/**
 * @brief
 * Handle basic capabilities
**/

// Common headers
#include "basic.hpp"

#include "../base/info.hpp"

// Session
#include "../session/SessionDebouncer.hpp"

// Configs
#include "../../configs/configs.hpp"

// Store
#include "../store/DocumentStore.hpp"

namespace Capabilities {
    static std::unique_ptr<Session::SessionDebouncer> debouncer = nullptr;

    lsp::ExecuteCommandOptions configureCommands(lsp::MessageHandler &messageHandler, Session::Session &session) {
        // Create the ExecuteCommandOptions struct
        lsp::ExecuteCommandOptions cmdOptions;

        // Register Juggernyaut's specific commands
        cmdOptions.commands = {
            "juggernyaut.server.session.trigger",
            "juggernyaut.server.session.rejuvenate"
        };

        messageHandler.add<lsp::requests::Workspace_ExecuteCommand>(
            [&session](const lsp::ExecuteCommandParams&& params) -> lsp::NullOr<lsp::LSPAny> {

                // Route the specific command
                if (params.command == "juggernyaut.server.session.trigger") {

                    debouncer->trigger();

                    return nullptr;
                } else if (params.command == "juggernyaut.server.session.rejuvenate") {

                    Session::rejuvenate(session);

                    return nullptr;
                }

                // If the client requested an unregistered command
                throw lsp::ResponseError(
                    (int) lsp::ErrorCodes::MethodNotFound,
                    "Command not recognized by Juggernyaut"
                );
            }
        );

        return cmdOptions;
    }

    void configureProtocol(lsp::MessageHandler &messageHandler, Session::Session &session, int &exit_code) {
        bool received_shutdown = false;
        Store::DocumentStore *store = static_cast<Store::DocumentStore*>(session.store);

        if (debouncer == nullptr) {
            debouncer = std::make_unique<Session::SessionDebouncer>(session);
        }

        messageHandler.add<lsp::requests::Initialize>(
            [&messageHandler, &session](lsp::requests::Initialize::Params&& params) {
                printMessage<lsp::requests::Initialize>(params);

                // Get the workspace's 'jug.toml' file
                if (!params.rootUri.isNull()) {
                    const std::string rootUri = std::string(params.rootUri.value().path());

                    // Look for 'jug.toml'
                    const std::string configUri = session.store->_joinPaths(rootUri, "jug.toml");
                    if (session.store->_isFileAccessible(configUri)) {
                        auto msgParams = lsp::notifications::Window_ShowMessage::Params{};

                        msgParams.type = lsp::MessageType::Info;
                        msgParams.message = "Juggernyaut configuration file has been imported: \n";
                        msgParams.message.append(configUri);

                        messageHandler.sendNotification<lsp::notifications::Window_ShowMessage>(std::move(msgParams));

                        // Load external configs
                        Configs::updateSessionConfigs(session, configUri);
                    }
                }

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
                        .executeCommandProvider = configureCommands(messageHandler, session)
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
            [store](lsp::notifications::TextDocument_DidOpen::Params&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                std::string sourceCode = std::move(params.textDocument.text);

                // Create doc
                store->addSource(rawUri, true);

                // Load doc
                store->syncRaw(rawUri, sourceCode);

                // Refresh the session
                debouncer->trigger();
            }
        ).add<lsp::notifications::TextDocument_DidChange>(
            [store](lsp::notifications::TextDocument_DidChange::Params&& params) {
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
                    debouncer->trigger();
                }
            }
        ).add<lsp::notifications::TextDocument_DidClose>(
            [store](lsp::notifications::TextDocument_DidClose::Params&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                // Load doc
                // TO-DO: Update content too??
                store->syncStatus(rawUri, false);

                // Refresh the session
                debouncer->trigger();
            }
        );
    }
}
