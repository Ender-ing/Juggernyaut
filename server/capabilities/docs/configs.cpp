/**
 * @brief
 * Manage configs onloading
**/

#include "configs.hpp"

#include "../semantics/diagnostics.hpp"

namespace Capabilities {
    namespace Docs {
        void registerConfigsWatcher(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver,
            std::unique_ptr<Driver::DriverDebouncer> &debouncer, const std::string &configUri) {
            // Add update trigger
            endpoint.onWorkspaceDidChangeWatchedFiles(
                [&driver, &debouncer](auto&& params) {
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

            // Push the watcher into the registration options
            //lsp::DidChangeWatchedFilesRegistrationOptions options;
            //options.watchers.push_back(watcher);
            //lsp::json::Object watcherObj;

            lsp::json::Object jugFilesWatcher, configFilesWatcher, configUriWatcher;
            jugFilesWatcher.insert("globPattern", "**/*.jug");
            configFilesWatcher.insert("globPattern", "**/jug.toml");
            configUriWatcher.insert("globPattern", driver.host.vfs->canonical(configUri));

            lsp::json::Array watchersArray;
            watchersArray.push_back(std::move(jugFilesWatcher));
            watchersArray.push_back(std::move(configFilesWatcher));
            watchersArray.push_back(std::move(configUriWatcher)); // In case it lives outside the workspace!
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
            endpoint.clientRegisterCapability(
                params,
                [](auto&& /*result*/) {
                    // Watcher successfully registered by the client
                },
                [&endpoint](const lsp::ResponseError&) {
                    endpoint.windowShowMessageRequest({
                        .type = lsp::MessageType::Warning,
                        .message = "Your workspace files are not being watched for changes. You may need to reload your editor in order for changes to take effect!",
                    });
                }
            );
        }
    }
}
