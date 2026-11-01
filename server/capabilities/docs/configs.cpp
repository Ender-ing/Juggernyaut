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
                            debouncer->trigger();
                        } else if (change.type == lsp::FileChangeType::Changed) {
                            driver.host.vfs->reloadFile((std::string) change.uri.path());
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
