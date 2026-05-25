/**
 * @brief
 * Manage file objects
**/

#include "files.hpp"

#include "FileStore.hpp"

#include "console/console.hpp"

namespace Store {
    bool FileStore::_getRawFile(const std::string &uri, std::string &output) const {
        if (!getFileContent(uri, output)) {
            output = "";
            return false;
        } else {
            return true;
        }
    }
    bool FileStore::_isFileAccessible(const std::string &uri) const {
        return isFileAccessible(uri);
    }
    bool FileStore::_isDirValid(const std::string &path) const {
        return isValidDir(path);
    }
    std::string FileStore::_getFileExtension(const std::string &uri) const {
        return getFileExtension(uri);
    }
    std::string FileStore::_getPathDir(const std::string &uri) const {
        return getParentPath(uri);
    }
    std::string FileStore::_getCanonical(const std::string &uri) const {
        return normalizePath(uri);
    }
    std::string FileStore::_joinPaths(const std::string &base, const std::string &path) const {
        return joinPaths(base, path);
    }
}
