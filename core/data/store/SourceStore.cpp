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
        void SourceStore::addEntry(SourceID entry) {
            std::vector<SourceID> &entries = this->entryPoints;
            if (std::ranges::find(entries, entry) == entries.end()) {
                entries.push_back(entry);
            }
        }
        void SourceStore::removeEntry(SourceID entry) {
            std::vector<SourceID> &entries = this->entryPoints;
            Common::Utility::fastVectorRemove(entries, entry);
        }
        void SourceStore::resetEntries() {
            std::vector<SourceID> &entries = this->entryPoints;
            entries.clear();
            entries.shrink_to_fit();
        }
        void SourceStore::visitEntries(const EntryCall entryCall) {
            std::vector<SourceID> &entries = this->entryPoints;
            for (auto entry : entries) {
                if (entryCall != nullptr) {
                    entryCall(entry);
                }
            }
        }

        // Sources
        std::unique_ptr<Source>& SourceStore::getSourceById(const SourceID &id) {
            std::unordered_map<SourceID, std::unique_ptr<Source>> &srcs = this->sources;

            return srcs.at(id);
        }
        void SourceStore::addSource(const std::string &uri, bool isEntry = false) {
            std::unordered_map<std::string, SourceID> &sourceStore = this->store;
            if (!sourceStore.contains(uri)) {
                std::unordered_map<SourceID, std::unique_ptr<Source>> &srcs = this->sources;

                // Create a <Source> object
                std::unique_ptr<Source> src = std::make_unique<Source>(uri, this);
                const SourceID &srcId = src->getID();

                if (isEntry) {
                    // Update <Source> data
                    src->setIsEntryPoint(true);
                    this->entryPoints.push_back(srcId);

                    // Keep track of the entry point
                    this->addEntry(srcId);
                }

                // Insert data
                srcs.insert({srcId, std::move(src)});
                sourceStore.insert({uri, srcId});
            }
        }

    }
}
