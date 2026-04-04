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
            std::string onFileRawRequest(const std::string &uri) override;
            bool resolvePath(const std::string &uri, std::string &output) override;
    };
}
