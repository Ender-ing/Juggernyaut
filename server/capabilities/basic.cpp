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

// Server Session Configs
#include "docs/configs.hpp"

namespace Capabilities {
    static std::unique_ptr<Session::SessionDebouncer> debouncer = nullptr;

    void logStdErr (std::string_view message) {
        std::cerr << "[DBG] EVENT: " << message << std::endl;
    }

    std::string configUri = "";
    void configureProtocol(lsp::ServerEndpoint &endpoint, Session::Session &session, int &exit_code) {
        bool received_shutdown = false;
        Store::DocumentStore *store = static_cast<Store::DocumentStore*>(session.store);

        if (debouncer == nullptr) {
            debouncer = std::make_unique<Session::SessionDebouncer>(session);
        }

        endpoint.onInitialize(
            [&session](auto&& params) -> lsp::InitializeResult {
                //printMessage<lsp::requests::Initialize>(params);
                logStdErr("Initialising the connection...");

                // Get the workspace's 'jug.toml' file
                if (!params.rootUri.isNull()) {
                    const std::string rootUri = std::string(params.rootUri.value().path());

                    // Look for 'jug.toml'
                    configUri = session.store->_joinPaths(rootUri, "jug.toml");
                }

                // Declare basic server info and capabilities
                return lsp::InitializeResult {
                    .capabilities = {
                        .positionEncoding = lsp::PositionEncodingKind::UTF16,
                        .textDocumentSync = lsp::TextDocumentSyncOptions {
                            .openClose = true,
                            .change = lsp::TextDocumentSyncKind::Full,
                            .save = true
                        },
                        .hoverProvider = false,
                        .definitionProvider = false,
                        .executeCommandProvider = lsp::ExecuteCommandOptions{
                            .commands = {
                                "juggernyaut.server.session.trigger",
                                "juggernyaut.server.session.rejuvenate"
                            }
                        }
                    },
                    .serverInfo = lsp::ServerInfo {
                        .name = "Juggernyaut Language Server",
                        .version = Base::Info::version
                    }
                };
            }
        ).onWorkspaceExecuteCommand(
            [&session](auto&& params) -> lsp::WorkspaceExecuteCommandResult {

                // Route the specific command
                if (params.command == "juggernyaut.server.session.trigger") {

                    debouncer->trigger();

                    return nullptr;
                } else if (params.command == "juggernyaut.server.session.rejuvenate") {

                    Session::rejuvenate(session);

                    return nullptr;
                }

                // If the client requested an unregistered command
                throw lsp::RequestError(lsp::MessageError::InvalidParams, "Unknown command: " + params.command);
            }
        ).onInitialized(
            [&session, &endpoint](auto&&) {
                logStdErr("Initialising the workspace configs...");

                std::thread([&session, &endpoint]() {
                    try {
                        if (configUri != "" && session.store->_isFileAccessible(configUri)) {
                            // Load external configs
                            Configs::BreakingChanges changes = Docs::updateSessionConfigs(endpoint, session, configUri);
                            Configs::refreshSessionState(session, changes);

                            // Watch file for changes
                            Docs::registerConfigsWatcher(endpoint, session, debouncer, configUri);
                        }

                        // Allow Session runs
                        debouncer->allowRuns = true;
                
                        // Optional: trigger the first debounce run now that everything is loaded
                        debouncer->trigger();
                
                    } catch (const std::exception& e) {
                        // If TOML parsing or something else fails, log it instead of crashing the server
                        std::cerr << "Initialization Error: " << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "Unknown Error during initialization thread." << std::endl;
                    }
                }).detach();
            }
        ).onShutdown(
            [&received_shutdown]() -> lsp::ShutdownResult {
                //printMessage<lsp::requests::Shutdown>();
                logStdErr("Shutting down the connection...");

                received_shutdown = true;
                return {};
            }
        ).onExit(
            [&received_shutdown, &exit_code]() {
                //printMessage<lsp::notifications::Exit>();
                logStdErr("Exiting...");

                exit_code = received_shutdown ? 0 : 1;
            }
        ).onTextDocumentDidOpen(
            [store](auto&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                std::string sourceCode = std::move(params.textDocument.text);

                // Create doc
                store->addSource(rawUri, true);

                // Load doc
                store->syncRaw(rawUri, sourceCode);

                // Refresh the session
                debouncer->trigger();
            }
        ).onTextDocumentDidChange(
            [store](auto&& params) {
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
        ).onTextDocumentDidClose(
            [store](auto&& params) {
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
