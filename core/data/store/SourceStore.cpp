/**
 * @brief 
 * "Source code" store
**/

#include "SourceStore.hpp"

#include "../../common/utility.hpp"

// Basic C++ libraries
#include <algorithm>

namespace Data {
    namespace Store {
        // Entry
        void SourceStore::addEntry(SourceId entry) {
            std::vector<SourceId> &entries = this->entryPoints;
            if (std::ranges::find(entries, entry) == entries.end()) {
                entries.push_back(entry);

                const std::unique_ptr<Source> &src = this->getSourceById(entry);
                src->setIsEntryPoint(true);
            }
        }
        void SourceStore::removeEntry(SourceId entry) {
            std::vector<SourceId> &entries = this->entryPoints;
            if (std::ranges::find(entries, entry) != entries.end()) {
                Common::Utility::fastVectorRemove(entries, entry);

                const std::unique_ptr<Source> &src = this->getSourceById(entry);
                src->setIsEntryPoint(false);
            }
        }
        void SourceStore::resetEntries() {
            std::vector<SourceId> &entries = this->entryPoints;
            entries.clear();
            entries.shrink_to_fit();
        }
        void SourceStore::visitEntries(const EntryCall entryCall) {
            std::vector<SourceId> &entries = this->entryPoints;
            for (auto entry : entries) {
                if (entryCall != nullptr) {
                    entryCall(entry);
                }
            }
        }

        // IDs
        SourceId SourceStore::getSourceIdByUri(const std::string &uri) {
            std::unordered_map<std::string, SourceId> &uriIndex = this->index;

            if (uriIndex.contains(uri)) {
                std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

                return uriIndex.contains(uri);
            } else {
                return 0;
            }
        }

        // Sources
        std::unique_ptr<Source>& SourceStore::getSourceById(const SourceId &id) {
            std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

            return srcs.at(id);
        }
        std::unique_ptr<Source>* SourceStore::getSourceByUri(const std::string &uri) {
            std::unordered_map<std::string, SourceId> &uriIndex = this->index;

            if (uriIndex.contains(uri)) {
                std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

                const SourceId &id = uriIndex.contains(uri);
                return &(srcs.at(id));
            } else {
                return nullptr;
            }
        }
        void SourceStore::addSource(const std::string &uri, bool isEntry = false) {
            std::unordered_map<std::string, SourceId> &uriIndex = this->index;
            if (!uriIndex.contains(uri)) {
                std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

                // Create a <Source> object
                std::unique_ptr<Source> src = std::make_unique<Source>(uri, this);
                const SourceId &srcId = src->getId();

                if (isEntry) {
                    // Update <Source> data
                    src->setIsEntryPoint(true);
                    this->entryPoints.push_back(srcId);

                    // Keep track of the entry point
                    this->addEntry(srcId);
                }

                // Insert data
                srcs.insert({srcId, std::move(src)});
                uriIndex.insert({uri, srcId});
            }
        }

    }
}
