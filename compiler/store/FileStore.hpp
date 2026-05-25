/**
 * @brief
 * Manage file objects
**/

#pragma once

// Common headers
#include "common/headers.hpp"

#include "../../core/data/store/SourceStore.hpp"

namespace Store {
    class FileStore : public Data::Store::SourceStore {
        private:
        public:
            using Data::Store::SourceStore::SourceStore;

            bool _getRawFile(const std::string &uri, std::string &output) const override;
            bool _isFileAccessible(const std::string &uri) const override;
            bool _isDirValid(const std::string &path) const override;
            std::string _getFileExtension(const std::string &uri) const override;
            std::string _getPathDir(const std::string &uri) const override;
            std::string _getCanonical(const std::string &uri) const override;
            std::string _joinPaths(const std::string &base, const std::string &path) const override;
    };
}
