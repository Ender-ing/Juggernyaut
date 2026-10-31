/**
 * @brief 
 * Virtual File System
**/

#include "VFS.hpp"

#include <filesystem>
// Make sure to only use <filesystem> functions that do not interact with the system/host!

namespace Data {
    namespace Host {
        // Technical
        File* VFS::addEmptyFile(const std::string& canonical) {
            // Generate a file object
            FilePtr file = std::make_unique<File>();
            file->canonical = canonical;

            // Update modification time
            if (this->trackModificationTime) {
                file->lastModified = this->_lastModified(canonical);
            }

            // Save the file
            FileId fileId = static_cast<FileId>(this->cache.size() + this->FILE_ID_OFFSET);
            file->id = fileId;
            this->cache.push_back(std::move(file));

            // Update URI index
            this->index[canonical] = fileId;

            return this->cache[fileId - this->FILE_ID_OFFSET].get();
        }

        // Internal
        void VFS::setFileContent(const std::string& path, std::string &newContent) {
            const std::string canonicalUri = this->_canonical(path);

            File* file;
            // Check if the file already exists
            auto it = this->index.find(canonicalUri);
            if (it != this->index.end()) {
                auto fileId = it->second;

                file = this->cache[fileId - this->FILE_ID_OFFSET].get();

                // Update modification time
                if (this->trackModificationTime) {
                    file->lastModified = this->_lastModified(canonicalUri);
                }
            } else {
                file  = this->addEmptyFile(canonicalUri);
            }

            // Update content
            file->content = std::move(newContent);
            newContent.clear(); // For safety!
            file->isValid = true;

            // Bump up the version
            file->version++;
        }
        void VFS::fetchLocalFileContent(const std::string& path) {
            const std::string canonicalUri = this->_canonical(path);

            File* file;
            // Check if the file already exists
            auto it = this->index.find(canonicalUri);
            if (it != this->index.end()) {
                auto fileId = it->second;

                file = this->cache[fileId - this->FILE_ID_OFFSET].get();

                // Update modification time
                if (this->trackModificationTime) {
                    file->lastModified = this->_lastModified(canonicalUri);
                }
            } else {
                file = this->addEmptyFile(canonicalUri);
            }

            // Update content
            file->isValid = this->_readFile(canonicalUri, file->content);

            // Bump up the version
            file->version++;
        }

        // External Management functions (Should not be called within the front-end!)
        // Note: newContent will be reset!
        void VFS::updateFile(const std::string& path, std::string &newContent) {
           std::lock_guard<std::mutex> lock(this->pendingMtx);
            if (this->safetyCount.load(std::memory_order_acquire) > 0) {
                // Driver is running: stage/coalesce the edit without touching `cache` or `index`
                this->pendingRequests[path] = FileChangeRequest{false, std::move(newContent)};
                newContent.clear();
                return;
            }
            this->setFileContent(path, newContent);
            newContent.clear();
        }
        void VFS::reloadFile(const std::string& path) {
            std::lock_guard<std::mutex> lock(this->pendingMtx);
            if (this->safetyCount.load(std::memory_order_acquire) > 0) {
                this->pendingRequests[path] = FileChangeRequest{true, ""};
                return;
            }
            this->fetchLocalFileContent(path);
        }
        const std::string VFS::getURI(const FileId& id) {
            const File* file = this->cache[id - this->FILE_ID_OFFSET].get();
            if (file == nullptr) {
                return "";
            }

            return file->canonical;
        }

        // Internal Management functions (Only called within the front-end!)
        void VFS::setSafeMode(bool safe) {
            std::lock_guard<std::mutex> lock(this->pendingMtx);
            // Enabling safe mode
            if (safe) {
                this->safetyCount.fetch_add(1, std::memory_order_release);
                return;
            }
            // Prevent extra calls
            uint32_t prev = this->safetyCount.load(std::memory_order_relaxed);
            if (prev == 0) return;

            // When opting out of safe mode, flush all queued operations!
            if (this->safetyCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                for (auto& [path, op] : this->pendingRequests) {
                    if (op.isReload) {
                        this->fetchLocalFileContent(path);
                    } else {
                        this->setFileContent(path, op.content);
                    }
                }
                this->pendingRequests.clear();
            }
        }
        bool VFS::isSafeMode() {
            return this->safetyCount.load() > 0;
        }

        // Basic functionality
        const File* VFS::readFile(const std::string& path) {
            const std::string canonicalUri = this->_canonical(path);

            // Check if the file already exists
            auto it = this->index.find(canonicalUri);
            if (it != this->index.end()) {
                auto fileId = it->second;

                return this->cache[fileId - this->FILE_ID_OFFSET].get();
            } else {
                // Generate a file object
                File* file = this->addEmptyFile(canonicalUri);

                // Request and load the file!
                // Diagnostic generation is handled by third parties!
                file->isValid = this->_readFile(canonicalUri, file->content);

                // Bump up the version
                file->version++;

                return file;
            }
        }
        const File* VFS::readFile(const FileId& id) {
            FileId indx = id - this->FILE_ID_OFFSET;
            if (id >= this->FILE_ID_OFFSET &&  indx < this->cache.size()) {
                return this->cache[indx].get();
            } else {
                return nullptr;
            }
        }
        bool VFS::exists(const std::string& path) {
            return this->_exists(this->_canonical(path));
        }
        bool VFS::isDirectory(const std::string& path) {
            return this->_isDirectory(this->_canonical(path));
        }
        const std::string VFS::canonical(const std::string& path) {
            return this->_canonical(path);
        }
        const std::string VFS::cwd() {
            return this->_cwd();
        }

        // Extended functionality
        const std::string VFS::getParentDirectory(const std::string& path) {
            return std::filesystem::path(path).parent_path().string();
        }
        const std::string VFS::join(const std::string &path, const std::string &append) {
            return this->_canonical((std::filesystem::path(path) / std::filesystem::path(append)).string());
        }
        bool VFS::endsWith(const std::string &path, const std::string &end) {
            if (path.length() >= end.length()) {
                return path.compare(path.length() - end.length(), end.length(), end) == 0;
            }
            return false;
        }
        const std::string VFS::filename(const std::string& path) {
            return std::filesystem::path(path).filename().string();
        }
    }
}
