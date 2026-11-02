/**
 * @brief
 * Handle basic capabilities
**/

// Common headers
#include "basic.hpp"

#include "../base/info.hpp"

// Front-End Driver
#include "../driver/DriverDebouncer.hpp"

// Server Session Configs
#include "docs/configs.hpp"

namespace Capabilities {
    static std::unique_ptr<Driver::DriverDebouncer> debouncer = nullptr;

    void logStdErr (std::string_view message) {
        std::cerr << "[DBG] EVENT: " << message << std::endl;
    }

    std::string rootUri = "",
        configUri = "";
    void configureProtocol(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver, int &exit_code) {
        bool received_shutdown = false;
        // Store::DocumentStore *store = static_cast<Store::DocumentStore*>(session.store);

        if (debouncer == nullptr) {
            debouncer = std::make_unique<Driver::DriverDebouncer>(driver);
        }

        endpoint.onInitialize(
            [](auto&& params) -> lsp::InitializeResult {
                //printMessage<lsp::requests::Initialize>(params);
                logStdErr("Initialising the connection...");

                // Get the workspace's 'jug.toml' file
                if (!params.rootUri.isNull()) {
                    rootUri = std::string(params.rootUri.value().path());
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
                                "juggernyaut.server.driver.trigger"
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
            [](auto&& params) -> lsp::WorkspaceExecuteCommandResult {

                // Route the specific command
                if (params.command == "juggernyaut.server.driver.trigger") {

                    debouncer->trigger();

                    return nullptr;
                }

                // If the client requested an unregistered command
                throw lsp::RequestError(lsp::MessageError::InvalidParams, "Unknown command: " + params.command);
            }
        ).onInitialized(
            [&driver, &endpoint](auto&&) {
                logStdErr("Initialising the workspace configs...");

                try {
                    if (rootUri == "") {
                        rootUri = driver.host.vfs->cwd();
                    }

                    configUri = driver.lookupProject(rootUri);

                    if (configUri != "") {
                        // Load up the project
                        driver.addProject(configUri);

                        // Watch the file for changes (In case the file was never opened!)
                        Docs::registerConfigsWatcher(endpoint, driver, debouncer, configUri);
                    }

                    // Allow Session runs
                    debouncer->allowRuns = true;

                    // Optional: trigger the first debounce run now that everything is loaded
                    debouncer->trigger();
                
                } catch (const std::exception& e) {
                    // If something fails, log it instead of crashing the server
                    std::cerr << "Initialization Error: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Unknown Error during initialization thread." << std::endl;
                }
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
            [&driver](auto&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());
                std::string sourceCode = std::move(params.textDocument.text);

                // Load data
                driver.host.vfs->updateFile(rawUri, sourceCode);

                // Create a project/sudo-project
                if (driver.host.vfs->filename(rawUri) != "jug.toml") {
                    driver.addStrayFile(rawUri);
                } else {
                    driver.addProject(rawUri);
                }

                // Refresh the session
                debouncer->trigger();
            }
        ).onTextDocumentDidChange(
            [&driver](auto&& params) {
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
                    driver.host.vfs->updateFile(rawUri, updatedSourceCode);

                    // Sync entry status!
                    // ...?

                    // Refresh the session
                    debouncer->trigger();
                }
            }
        ).onTextDocumentDidClose(
            [&driver](auto&& params) {
                const std::string rawUri = std::string(params.textDocument.uri.path());

                // Reset content
                driver.host.vfs->reloadFile(rawUri);

                // Sync entry status!
                // ...?

                // Refresh the session
                debouncer->trigger();
            }
        );
    }
}
