/**
 * @brief
 * Manage document objects and content updates
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

#include "./../../core/data/store/SourceStore.hpp"

namespace Store {
    class DocumentStore : public Data::Store::SourceStore {
        private:
            std::unordered_map<std::string, std::string> syncedRaws;
        public:
            // LSP synchronisation
            void syncRaw(const std::string &uri, const std::string &rawContent) ;
            void syncStatus(const std::string &uri, bool isInEditor) ;

            std::string onFileRawRequest(const std::string &uri) override;
            bool resolvePath(const std::string &uri, std::string &output) override;
    };
}
