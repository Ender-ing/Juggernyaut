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

    std::string DocumentStore::onFileRawRequest(const std::string &uri) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        if (raws.contains(uri)) {
            return raws.at(uri);
        } else {
            if (Store::isFileAccessible(uri) && Store::isFileValid(uri)) {
                std::string content;
                if (Store::getFileContent(uri, content)) {
                    return content;
                }
            }
            return std::string("");
        }
    }
    bool DocumentStore::resolvePath(const std::string &uri, std::string &output) {
        if (!Store::isFileAccessible(uri)){
            output = "file is inaccessible";
            return false;
        } else if (!Store::isFileValid(uri)) {
            output = "file is invalid";
            return false;
        }

        output = lsp::DocumentUri::fromPath(uri).path();
        return true;
    }
}
