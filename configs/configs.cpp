/**
 * @brief
 * Handle .toml configs
**/

#include "configs.hpp"

// toml++
#include "tomlpp.hpp"

namespace Configs {
    bool modifySession(Session::Session &session, const std::string &configUri, std::string &errorLog) {
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

        toml::table &table = result.table();
        table;
        // ...

        return true;
    }
}
