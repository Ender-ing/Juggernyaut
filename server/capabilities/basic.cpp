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

    static std::string rootUri = "",
        configUri = "";
    static bool received_shutdown = false;
    static lsp::PositionEncodingKindEnum activeEncoding = lsp::PositionEncodingKind::UTF16;
    void configureProtocol(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver, int &exit_code) {
        received_shutdown = false;

        // Always reset the debouncer
        debouncer = std::make_unique<Driver::DriverDebouncer>(driver);

        endpoint.onInitialize(
            [](auto&& params) -> lsp::InitializeResult {
                //printMessage<lsp::requests::Initialize>(params);
                logStdErr("Initialising the connection...");

                // Get the workspace's 'jug.toml' file
                if (!params.rootUri.isNull()) {
                    rootUri = std::string(params.rootUri.value().path());
                }

                // Check for UTF8 support
                activeEncoding = lsp::PositionEncodingKind::UTF16;
                if (params.capabilities.general.has_value() &&
                    params.capabilities.general->positionEncodings.has_value()) {
                    for (const auto& enc : *params.capabilities.general->positionEncodings) {
                        if (enc == lsp::PositionEncodingKind::UTF8) {
                            activeEncoding = lsp::PositionEncodingKind::UTF8;
                            break;
                        }
                    }
                }

                // Declare basic server info and capabilities
                return lsp::InitializeResult {
                    .capabilities = {
                        .positionEncoding = activeEncoding,
                        .textDocumentSync = lsp::TextDocumentSyncOptions {
                            .openClose = true,
                            .change = lsp::TextDocumentSyncKind::Incremental,
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
            []() -> lsp::ShutdownResult {
                //printMessage<lsp::requests::Shutdown>();
                logStdErr("Shutting down the connection...");

                received_shutdown = true;
                return {};
            }
        ).onExit(
            [&exit_code]() {
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
                if (params.contentChanges.empty()) {
                    return;
                }

                // Prepare for edits conversion
                const std::string rawUri = std::string(params.textDocument.uri.path());
                const Data::Host::File* file = driver.host.vfs->readFile(rawUri);
                std::vector<Data::Host::ByteEdit> edits;
                edits.reserve(params.contentChanges.size());

                // Handle UTF16 to UTF8 conversion
                auto toByteOffset = [&](const lsp::Position& pos) -> Data::Host::FileByte {
                    Data::Host::FileByte bytePos = file->content.lineStartByte(pos.line);
                    if (activeEncoding == lsp::PositionEncodingKind::UTF8) {
                        return bytePos + pos.character;
                    }

                    Data::Host::FileByte total = file->content.byteLength();
                    Data::Host::FileByte u16 = 0;
                    while (bytePos < total && u16 < pos.character) {
                        std::string_view chunk = file->content.getChunk(bytePos);
                        if (chunk.empty()) break;

                        unsigned char c = static_cast<unsigned char>(chunk[0]);
                        if (c == '\n' || c == '\r') break;

                        Data::Host::FileByte charBytes =
                            (c < 0x80) ? 1 : ((c & 0xE0) == 0xC0) ? 2 : ((c & 0xF0) == 0xE0) ? 3 : 4;
                        Data::Host::FileByte u16Units = (charBytes == 4) ? 2 : 1;
                        if (u16 + u16Units > pos.character) break;

                        u16 += u16Units;
                        bytePos += charBytes;
                    }
                    return bytePos;
                };

                // Feed changes into 'edits'
                for (auto& change : params.contentChanges) {
                    std::visit(
                        [&](auto& changeEvent) {
                            using T = std::decay_t<decltype(changeEvent)>;
                            if constexpr (std::is_same_v<T, lsp::TextDocumentContentChangePartial>) {
                                edits.push_back(Data::Host::ByteEdit{
                                    toByteOffset(changeEvent.range.start),
                                    toByteOffset(changeEvent.range.end),
                                    std::move(changeEvent.text)
                                });
                            } else {
                                edits.clear();
                                driver.host.vfs->updateFile(rawUri, changeEvent.text);
                            }
                        },
                        change
                    );
                }

                // Apply edits
                if (!edits.empty()) {
                    driver.host.vfs->editFile(rawUri, edits);
                }

                // Sync entry status!
                // ...?

                // Refresh the session
                debouncer->trigger();
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
        ).onWorkspaceDidChangeWatchedFiles(
            [&driver](auto&& params) {
                for (const auto& change : params.changes) {
                    if (change.type == lsp::FileChangeType::Deleted) {
                        std::string emptyStr = "";
                        driver.host.vfs->updateFile((std::string) change.uri.path(), emptyStr);
                    } else if (change.type == lsp::FileChangeType::Changed || change.type == lsp::FileChangeType::Created) {
                        driver.host.vfs->reloadFile((std::string) change.uri.path());
                    }
                }
                debouncer->trigger();
            }
        );
    }
    void shutdownProtocol() {
        debouncer.reset();
    }
}
