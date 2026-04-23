/**
 * @brief
 * Handle .toml configs
**/

#include "configs.hpp"

// toml++
#include "tomlpp.hpp"

namespace Configs {
    namespace Internal {
        bool getTable(Session::Session &session, const std::string &configUri, std::string &errorLog, toml::table &table) {
            if (session.store == nullptr) {
                errorLog = "no store object";
                return false;
            }

            std::string rawContent;
            if (!(session.store->_getRawFile(configUri, rawContent))) {
                errorLog = "no store object";
                return false;
            }

            // Process toml data
            toml::parse_result result = toml::parse(rawContent);
            if (!result) {
                std::stringstream details;
                details << "parsing failed:\n    "
                    << result.error().description()
                    << "\n    ("
                    << result.error().source().begin
                    << ")";

                errorLog = details.str();
                return false;
            }
            rawContent.clear();
            rawContent.shrink_to_fit();

            table = result.table();
            return true;
        }
    }
    bool modifySession(Session::Session &session, const std::string &configUri, std::string &errorLog, bool isStrict) {
        toml::table configs;
        const std::string rootUri = session.store->_getPathDir(configUri);

        Internal::getTable(session, configUri, errorLog, configs);

        if (configs.contains("workspace")) {
            if (configs["workspace"]["importDirs"].is_array()) {
                toml::array* dirs = configs["workspace"]["importDirs"].as_array();

                if (isStrict) {
                    session.store->resetImportDirs();
                }

                for (toml::node& dir : *dirs) {
                    if (dir.is_string()) {
                        std::string dirStr = dir.value_or("");

                        std::string absPath = session.store->_joinPaths(rootUri, dirStr);
                        session.store->addImportDir(absPath);
                    }
                }
            }
        }

        return true;
    }
}
