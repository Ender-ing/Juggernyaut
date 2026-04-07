/**
 * @brief
 * Manage file objects
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

#include "../../core/data/store/SourceStore.hpp"

namespace Store {
    class FileStore : public Data::Store::SourceStore {
        private:
        public:
            bool _getRawFile(const std::string &uri, std::string &output) override;
            bool _isFileAccessible(const std::string &uri) override;
            std::string _getFileExtension(const std::string &uri) override;
            std::string _getPathDir(const std::string &uri) override;
            std::string _getCanonical(const std::string &uri) override;
    };
}
