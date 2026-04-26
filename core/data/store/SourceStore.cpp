/**
 * @brief 
 * "Source code" store
**/

#include "SourceStore.hpp"

#include "common/utility.hpp"

// Basic C++ libraries
#include <algorithm>

namespace Data {
    namespace Store {
        // Import paths
        void SourceStore::addImportDir(const std::string &path) {
            std::vector<std::string> &paths = this->importDirs;

            if (this->_isDirValid(path)) {
                if (!Common::Utility::isInVector(paths, path)) {
                    paths.push_back(path);
                }
            }
        }
        void SourceStore::resetImportDirs() {
            std::vector<std::string> &paths = this->importDirs;

            paths.clear();
            paths.shrink_to_fit();
        }

        // General path lookup
        bool SourceStore::resolvePath(const std::string &uri, std::string &output, SourceId callerId) {
            std::string path = uri;
            if (this->_getFileExtension(path) != ".jug") {
                path.append(".jug");
            }

            // Relative path
            if (callerId != 0) { // To a file
                std::unique_ptr<Source> &src = this->getSourceById(callerId);

                std::string relativePath = this->_joinPaths(this->_getPathDir(src->uri), path);

                if (this->_isFileAccessible(relativePath)) {
                    output = this->_getCanonical(relativePath);
                    return true;
                }
            } else { // to the running directory
                if (this->_isFileAccessible(path)) {
                    output = this->_getCanonical(path);
                    return true;
                }
            }

            std::string importDir;
            for (const auto &dir : this->importDirs) {
                importDir = this->_joinPaths(dir, path);
                if (this->_isFileAccessible(importDir)) {
                    output = this->_getCanonical(importDir);
                    return true;
                }
            }

            output = "file is inaccessible";
            return false;
        }

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
            if (entryCall != nullptr) {
                std::vector<SourceId> &entries = this->entryPoints;
                for (auto &entry : entries) {
                    entryCall(entry);
                }
            }
        }

        // IDs
        SourceId SourceStore::getSourceIdByUri(const std::string &uri) {
            std::unordered_map<std::string, SourceId> &uriIndex = this->index;

            const std::string canonical = this->_getCanonical(uri);

            if (uriIndex.contains(canonical)) {
                return uriIndex.at(canonical);
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

            const std::string canonical = this->_getCanonical(uri);

            if (uriIndex.contains(canonical)) {
                std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

                const SourceId &id = uriIndex.at(canonical);
                return &(srcs.at(id));
            } else {
                return nullptr;
            }
        }
        void SourceStore::addSource(const std::string &uri, bool isEntry = false) {
            std::unordered_map<std::string, SourceId> &uriIndex = this->index;


            
            std::cerr << "OLD-NEW: " << uri << this->_getCanonical(uri) << std::endl;
            std::cerr << "TEST: " << "./test.jug" << this->_getCanonical("./test.jug") << std::endl;



            const std::string canonical = this->_getCanonical(uri);

            if (!uriIndex.contains(canonical)) {
                std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

                // Create a <Source> object
                std::unique_ptr<Source> src = std::make_unique<Source>(canonical, this);
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
                uriIndex.insert({canonical, srcId});
            }
        }

        // Memory housekeeping
        static uint32_t currentRound = 0;
        void SourceStore::deleteSource(std::unique_ptr<Source> &source, bool erase) {
            std::unordered_map<std::string, SourceId> &srcIndex = this->index;
            std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;

            // Get data
            const SourceId srcId = source->getId();
            const std::string &uri = source->uri;

            // Remove traces
            if (srcIndex.contains(uri)) {
                srcIndex.erase(uri);
            }

            // Remove entry point
            this->removeEntry(srcId);

            // Delete element
            source.reset();
            if (erase && srcs.contains(srcId)) {
                srcs.erase(srcId);
            }
        }
        void SourceStore::cleanup() {
            SourceStore *srcStore = this;

            // Begin a new round!
            currentRound++;

            // Update accessible entries
            this->visitEntries([&srcStore](SourceId srcId) {
                std::unique_ptr<Data::Store::Source> &src = srcStore->getSourceById(srcId);

                src->round = currentRound;
            });

            // Discard inaccessible entries
            std::unordered_map<SourceId, std::unique_ptr<Source>> &srcs = this->sources;
            auto it = srcs.begin();
            while (it != srcs.end()) {
                if (it->second->round != currentRound) {
                    std::cerr << "AAAA: " << it->second->round << "-" << it->second->getId() << "-" << it->second->uri << std::endl;
                    for (auto a:this->entryPoints) {
                        std::cerr << "BBBB: " << a << std::endl;
                    }
                    srcStore->deleteSource(it->second, false);
                    it = srcs.erase(it); // Erase, and fetch!
                } else {
                    ++it; // Skip
                }
            }
        }
    }
}
