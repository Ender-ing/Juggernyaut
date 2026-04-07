/**
 * @brief
 * Manage document objects and content updates
**/

#include "DocumentStore.hpp"

// Capabilities
#include "../capabilities/basic.hpp"

// lsp-framework
#include "../lspFramework.hpp"

#include "files.hpp"

namespace Store {
    // LSP synchronisation
    void DocumentStore::syncRaw(const std::string &uri, const std::string &rawContent) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        if (raws.contains(uri)) {
            raws.at(uri) = rawContent;
        } else {
            raws.insert({uri, rawContent});
        }

        // Invalidate file raw file content
        std::unique_ptr<Data::Store::Source> *srcPtr = this->getSourceByUri(uri);
        if (srcPtr != nullptr) {
            std::unique_ptr<Data::Store::Source> &src = *srcPtr;
            src->invalidateRawContent();
        }
    }
    void DocumentStore::syncStatus(const std::string &uri, bool isInEditor) {
        const Data::Store::SourceId &srcId = this->getSourceIdByUri(uri);

        if (srcId != 0) {
            if (isInEditor) {
                this->addEntry(srcId);
            } else {
                this->removeEntry(srcId);
            }
        }
    }

    bool DocumentStore::_getRawFile(const std::string &uri, std::string &output) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        if (raws.contains(uri)) {
            output = raws.at(uri);
            return true;
        } else {
            if (!Store::getFileContent(uri, output)) {
                output = "";
                return false;
            } else {
                return true;
            }
        }
    }
    bool DocumentStore::_isFileAccessible(const std::string &uri) {
        return isFileAccessible(uri);
    }
    std::string DocumentStore::_getFileExtension(const std::string &uri) {
        return getFileExtension(uri);
    }
    std::string DocumentStore::_getPathDir(const std::string &uri) {
        return getParentPath(uri);
    }
    std::string DocumentStore::_getCanonical(const std::string &uri) {
        return normalizePath(uri);
    }
    std::string DocumentStore::_joinPaths(const std::string &base, const std::string &path) {
        return joinPaths(base, path);
    }

    void DocumentStore::deleteSource(std::unique_ptr<Data::Store::Source> &src) {
        Data::Store::SourceStore::deleteSource(src);

        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        // Delete file data
        const std::string &uri = src->uri;
        if (raws.contains(uri)) {
            raws.erase(uri);
        }
    }
}
