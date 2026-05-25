/**
 * @brief
 * Manage document objects and content updates
**/

#include "DocumentStore.hpp"

// Capabilities
#include "../capabilities/basic.hpp"
#include "../capabilities/semantics/diagnostics.hpp"

// lsp-framework
#include "../lspFramework.hpp"

#include "files.hpp"

namespace Store {
    // LSP synchronisation
    void DocumentStore::syncRaw(const std::string &uri, const std::string &rawContent) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        const std::string canonical = this->_getCanonical(uri);

        if (raws.contains(canonical)) {
            raws.at(canonical) = rawContent;
        } else {
            raws.insert({canonical, rawContent});
        }

        // Invalidate raw content
        std::unique_ptr<Data::Store::Source> *srcPtr = this->getSourceByURI(canonical);
        if (srcPtr != nullptr) {
            std::unique_ptr<Data::Store::Source> &src = *srcPtr;
            src->invalidateRawContent();
        }
    }
    void DocumentStore::syncStatus(const std::string &uri, bool isInEditor) {
        const Data::Store::SourceId &srcId = this->getSourceIdByURI(uri);

        if (srcId != 0) {
            if (isInEditor) {
                this->addEntry(srcId);
            } else {
                this->removeEntry(srcId);
            }
        }
    }

    bool DocumentStore::_getRawFile(const std::string &uri, std::string &output) const {
        const std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

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
    bool DocumentStore::_isFileAccessible(const std::string &uri) const {
        return isFileAccessible(uri);
    }
    bool DocumentStore::_isDirValid(const std::string &path) const {
        return isValidDir(path);
    }
    std::string DocumentStore::_getFileExtension(const std::string &uri) const {
        return getFileExtension(uri);
    }
    std::string DocumentStore::_getPathDir(const std::string &uri) const {
        return getParentPath(uri);
    }
    std::string DocumentStore::_getCanonical(const std::string &uri) const {
        return normalizePath(uri);
    }
    std::string DocumentStore::_joinPaths(const std::string &base, const std::string &path) const {
        return joinPaths(base, path);
    }

    void DocumentStore::deleteSource(std::unique_ptr<Data::Store::Source> &src, bool erase) {
        // Get data
        const std::string uri = src->uri;

        Data::Store::SourceStore::deleteSource(src, erase);

        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        // Reset diagnostics
        Capabilities::Semantics::resetSourceDiagnostics(uri);

        // Delete file data
        if (raws.contains(uri)) {
            raws.erase(uri);
        }
    }
}
