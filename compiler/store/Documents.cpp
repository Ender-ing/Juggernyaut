/**
 * @brief 
 * Virtual File System
**/

#include "Documents.hpp"

#include "files.hpp"

namespace Store {
    // Local system binds
    bool Documents::_readFile(const std::string& path, std::string& output) {
        if (!getFileContent(path, output)) {
            output.clear();
            return false;
        }
        return true;
    }
    bool Documents::_exists(const std::string& path) {
        return isFileAccessible(path) || isValidDir(path);
    }
    bool Documents::_isDirectory(const std::string& path) {
        return isValidDir(path);
    }
    const std::string Documents::_canonical(const std::string& path) {
        return normalizePath(path);
    }
    Timestamp Documents::_lastModified(const std::string& path) {
        (void)path;
        return 0;
    }
    const std::string Documents::_cwd() {
        return getWorkingDir();
    }
}
