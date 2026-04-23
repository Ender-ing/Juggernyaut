/**
 * @brief
 * Manage document objects and content updates
**/

#pragma once

// Common headers
#include "common/headers.hpp"

#include "./../../core/data/store/SourceStore.hpp"

namespace Store {
    class DocumentStore : public Data::Store::SourceStore {
        private:
            std::unordered_map<std::string, std::string> syncedRaws;
        public:
            // LSP synchronisation
            void syncRaw(const std::string &uri, const std::string &rawContent) ;
            void syncStatus(const std::string &uri, bool isInEditor) ;

            bool _getRawFile(const std::string &uri, std::string &output) override;
            bool _isFileAccessible(const std::string &uri) override;
            bool _isDirValid(const std::string &path) override;
            std::string _getFileExtension(const std::string &uri) override;
            std::string _getPathDir(const std::string &uri) override;
            std::string _getCanonical(const std::string &uri) override;
            std::string _joinPaths(const std::string &base, const std::string &path) override;

            void deleteSource(std::unique_ptr<Data::Store::Source> &src) override;
    };
}
