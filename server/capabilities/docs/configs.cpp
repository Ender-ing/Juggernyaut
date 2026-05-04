/**
 * @brief
 * Manage configs onloading
**/

#include "configs.hpp"

#include "../semantics/diagnostics.hpp"

namespace Capabilities {
    namespace Docs {
        Configs::BreakingChanges updateSessionConfigs(lsp::MessageHandler &messageHandler, Session::Session &session, const std::string &configUri) {
            const std::string uri = session.store->_getCanonical(configUri);

            auto msgParams = lsp::notifications::Window_ShowMessage::Params{};
            msgParams.type = lsp::MessageType::Info;
            msgParams.message = "Juggernyaut configuration file is being processed...";
            messageHandler.sendNotification<lsp::notifications::Window_ShowMessage>(std::move(msgParams));

            // Load external configs
            std::vector<Diagnostics::Diagnostic> configsDiags;
            Configs::BreakingChanges changes = Configs::BreakingChanges::None;
            if (!Configs::modifySession(session, uri, configsDiags, true, &changes)) {

                std::erase_if(configsDiags, [&messageHandler](const Diagnostics::Diagnostic &diag) {
                    // Take care of sources with no range!
                    if (diag.range.start == diag.range.end) {
                        auto errorParams = lsp::notifications::Window_ShowMessage::Params{};

                        errorParams.type = lsp::MessageType::Error;
                        errorParams.message = "Juggernyaut configuration file error: \n";
                        errorParams.message.append(std::move(diag.message));

                        messageHandler.sendNotification<lsp::notifications::Window_ShowMessage>(std::move(errorParams));
                        return true;
                    }
                    return false;
                });
            }

            // (Special Case) send diagnostics
            Capabilities::Semantics::sendSourceDiagnosticsByURI(configsDiags, uri);

            return changes;
        }
        void registerConfigsWatcher(lsp::MessageHandler &messageHandler, Session::Session &session,
            std::unique_ptr<Session::SessionDebouncer> &debouncer, const std::string &configUri) {
            // Add update trigger
            messageHandler.add<lsp::notifications::Workspace_DidChangeWatchedFiles>(
                [&messageHandler, &session, &debouncer](lsp::notifications::Workspace_DidChangeWatchedFiles::Params&& params) {
                    for (const auto& change : params.changes) {
                        if (change.type == lsp::FileChangeType::Deleted) {
                            Configs::BreakingChanges changes = Configs::resetSessionConfigs(session);
                            Configs::refreshSessionState(session, changes);
                            debouncer->trigger();
                        } else if (change.type == lsp::FileChangeType::Changed) {
                            Configs::BreakingChanges changes = Docs::updateSessionConfigs(messageHandler, session, (std::string) change.uri.path());
                            Configs::refreshSessionState(session, changes);
                            debouncer->trigger();
                        }
                    }
                }
            );

            // File watcher rule
            lsp::FileSystemWatcher watcher;
            watcher.globPattern = configUri;

            lsp::DidChangeWatchedFilesRegistrationOptions options;
            options.watchers.push_back(std::move(watcher));

            // Create the registration entry
            lsp::Registration registration;
            registration.id = "watch-config-file-id"; 
            registration.method = "workspace/didChangeWatchedFiles";
    
            // Note: Depending on how lspgen generates the struct, registerOptions 
            // might be a std::any, json::Value, or specifically typed. 
            // If it's json::Value, you'd serialize 'options' to JSON here.
            registration.registerOptions = lsp::toJson(std::move(options));

            lsp::requests::Client_RegisterCapability::Params params;
            params.registrations.push_back(std::move(registration));

            // Send the request to the client
            messageHandler.sendRequest<lsp::requests::Client_RegisterCapability>(
                std::move(params),
                [](lsp::requests::Client_RegisterCapability::Result&&) {
                    // SUCCESS!
                },
                [&messageHandler](const lsp::ResponseError& error) { // On Error
                    auto errorParams = lsp::notifications::Window_ShowMessage::Params{};

                    errorParams.type = lsp::MessageType::Error;
                    errorParams.message = "Failed to set up Juggernyaut configuration file watcher! You may need to reload your current editor. Details: ";
                    errorParams.message.append(error.message());

                    messageHandler.sendNotification<lsp::notifications::Window_ShowMessage>(std::move(errorParams));
                }
            );
        }
    }
}
