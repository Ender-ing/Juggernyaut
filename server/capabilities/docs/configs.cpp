/**
 * @brief
 * Manage configs onloading
**/

#include "configs.hpp"

#include "../semantics/diagnostics.hpp"

namespace Capabilities {
    namespace Docs {
        Configs::BreakingChanges updateSessionConfigs(lsp::ServerEndpoint &endpoint, Session::Session &session, const std::string &configUri) {
            const std::string uri = session.store->_getCanonical(configUri);

            (void)endpoint.windowShowMessageRequest(
                {
                    .type = lsp::MessageType::Info,
                    .message = "Juggernyaut configuration file is being processed...",
                }
            );

            // Load external configs
            std::vector<Diagnostics::Diagnostic> configsDiags;
            Configs::BreakingChanges changes = Configs::BreakingChanges::None;
            if (!Configs::modifySession(session, uri, configsDiags, true, &changes)) {

                std::erase_if(configsDiags, [&endpoint](const Diagnostics::Diagnostic &diag) {
                    // Take care of sources with no range!
                    if (diag.range.start == diag.range.end) {
                        std::string msg = "Juggernyaut configuration file is being processed...";
                        msg.append(std::move(diag.message));

                        (void)endpoint.windowShowMessageRequest(
                            {
                                .type    = lsp::MessageType::Error,
                                .message = std::move(msg),
                            }
                        );

                        return true;
                    }
                    return false;
                });
            }

            // (Special Case) send diagnostics
            Capabilities::Semantics::sendSourceDiagnosticsByURI(configsDiags, uri);

            return changes;
        }
        void registerConfigsWatcher(lsp::ServerEndpoint &endpoint, Session::Session &session,
            std::unique_ptr<Session::SessionDebouncer> &debouncer, const std::string &configUri) {
            // Add update trigger
            endpoint.onWorkspaceDidChangeWatchedFiles(
                [&endpoint, &session, &debouncer](auto&& params) {
                    for (const auto& change : params.changes) {
                        if (change.type == lsp::FileChangeType::Deleted) {
                            Configs::BreakingChanges changes = Configs::resetSessionConfigs(session);
                            Configs::refreshSessionState(session, changes);
                            debouncer->trigger();
                        } else if (change.type == lsp::FileChangeType::Changed) {
                            Configs::BreakingChanges changes = Docs::updateSessionConfigs(endpoint, session, (std::string) change.uri.path());
                            Configs::refreshSessionState(session, changes);
                            debouncer->trigger();
                        }
                    }
                }
            );

            // Configure the File System Watcher
            lsp::FileSystemWatcher watcher;
            watcher.globPattern = configUri;

            // Push the watcher into the registration options
            //lsp::DidChangeWatchedFilesRegistrationOptions options;
            //options.watchers.push_back(watcher);
            lsp::json::Object watcherObj;
            watcherObj.insert("globPattern", "/*.txt");
            lsp::json::Array watchersArray;
            watchersArray.push_back(watcherObj);
            lsp::json::Object optionsObj;
            optionsObj.insert("watchers", std::move(watchersArray));

            // Configure registration request
            lsp::Registration registration;
            registration.id = "workspace-configs-file-watcher";
            registration.method = lsp::notifications::WorkspaceDidChangeWatchedFiles::Method;
            registration.registerOptions = std::move(optionsObj);

            // Add to Registration Params
            lsp::RegistrationParams params;
            params.registrations.push_back(registration);

            // Send request to client
            auto requestResult = endpoint.clientRegisterCapability(params);
            std::thread([&endpoint, result = std::move(requestResult)]() mutable {
                try {
                    // .get() resolves the std::future or executes the TaskType
                    (void)result.get(); 
        
                    // The client successfully registered the watcher
                } catch (const std::exception&) {
                    (void)endpoint.windowShowMessageRequest(
                        {
                            .type = lsp::MessageType::Warning,
                            .message = "Your workspace Juggernyaut configuration file is not being watched for changes. You may need to reload your editor in order for changes to take effect!",
                        }
                    );
                }
            }).detach();
        }
    }
}
