/**
 * @brief
 * Manage file objects
**/

#include "files.hpp"

#include "FileStore.hpp"

#include "../console/console.hpp"

namespace Store {
    bool FileStore::_getRawFile(const std::string &uri, std::string &output) {
        if (!getFileContent(uri, output)) {
            output = "";
            return false;
        } else {
            return true;
        }
    }
    bool FileStore::_isFileAccessible(const std::string &uri) {
        return isFileAccessible(uri);
    }
    std::string FileStore::_getFileExtension(const std::string &uri) {
        return getFileExtension(uri);
    }
    std::string FileStore::_getPathDir(const std::string &uri) {
        return getParentPath(uri);
    }
    std::string FileStore::_getCanonical(const std::string &uri) {
        return normalizePath(uri);
    }
}
