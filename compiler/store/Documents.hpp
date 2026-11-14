/**
 * @brief
 * Virtual File System
**/

#pragma once

// Common headers
#include "common/headers.hpp"

#include "../../core/data/host/VFS.hpp"

namespace Store {
    using VFS = Data::Host::VFS;
    using Timestamp = Data::Host::Timestamp;

    class Documents : public Data::Host::VFS {
        protected:
            // Local system binds
            bool _readFile(const std::string& path, std::string& output) override;
            bool _exists(const std::string& path) override;
            bool _isDirectory(const std::string& path) override;
            const std::string _canonical(const std::string& path) override; // Returns the canonical full path of a file
            Timestamp _lastModified(const std::string& path) override;
            const std::string _cwd() override;
    };
}
