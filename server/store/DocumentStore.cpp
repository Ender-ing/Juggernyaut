/**
 * @brief
 * Manage document objects and content updates
**/

#include "DocumentStore.hpp"

// Capabilities
#include "../capabilities/basic.hpp"

// lsp-framework
#include "../lspFramework.hpp"

namespace Store {
    // LSP synchronisation
    void DocumentStore::syncRaw(const std::string &uri, const std::string &rawContent) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        if (raws.contains(uri)) {
            raws.at(uri) = rawContent;
        } else {
            raws.insert({uri, rawContent});
        }
    }
    void DocumentStore::syncStatus(const std::string &uri, bool isInEditor) {
        std::unordered_map<std::string, Data::Store::SourceId> &index = this->index;

        const Data::Store::SourceId &srcId = this->getSourceIdByUri(uri);

        if (srcId != 0) {
            this->addEntry(srcId);
        }
    }

    std::string DocumentStore::onFileRawRequest(const std::string &uri) {
        std::unordered_map<std::string, std::string> &raws = this->syncedRaws;

        if (raws.contains(uri)) {
            return raws.at(uri);
        } else {
            return std::string("TMP: COULDN'T SYNC!");
        }
    }
}
