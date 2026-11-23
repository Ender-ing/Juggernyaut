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
        File* VFS::getFilePtr(const std::string& path) {
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

            // Return pointer
            return file;
        }
        void VFS::setFileContent(const std::string& path, std::string &newContent) {
            std::unique_lock<std::shared_mutex> writeLock(this->cacheMtx);

            File* file = getFilePtr(path);

            // Update content & internal state
            file->content.setFullContent(newContent);

            // Bump up the version
            file->version++;

            // Update internal state
            file->pendingDeltas.clear();
            file->deltasBaseVersion = file->version;
            file->isValid = true;
        }
        constexpr size_t MAX_PENDING_DELTAS = 128;
        void VFS::editFileContent(const std::string& path, std::vector<ByteEdit> &edits) {
            if (edits.empty()) {
                return;
            }
            std::unique_lock<std::shared_mutex> writeLock(this->cacheMtx);

            File* file = getFilePtr(path);

            // Keep 'deltasBaseVersion' up-to-date
            if (file->pendingDeltas.empty()) {
                file->deltasBaseVersion = file->version;
                this->dirtyDeltaFiles.push_back(file);
            }

            // Bump up the version
            file->version++;

            // If no CST exists yet, or the CST already consumed all deltas up to
            // file->version, clear old deltas before recording the new batch.
            /*if (file->cst == nullptr || file->cstVersion == file->version) {
                file->pendingDeltas.clear();
            }*/

            // Update content
            for (const ByteEdit& edit : edits) {
                file->pendingDeltas.push_back( // Keep track of changes for CST regeneration!
                    file->content.applyEdit(edit.startByte, edit.oldEndByte, edit.text) // Apply edit
                );
            }
            file->content.compact();
            file->isValid = true;

            // Guard against single burst dumps
            if (file->pendingDeltas.size() > MAX_PENDING_DELTAS) {
                file->pendingDeltas.clear();
                file->deltasBaseVersion = file->version; // Fall back to a full re-parse
            }
        }
        void VFS::fetchLocalFileContent(const std::string& path) {
            // Read from disk outside `cacheMtx`
            const std::string canonicalUri = this->_canonical(path);
            std::string newContent;
            const bool isValid = this->_readFile(canonicalUri, newContent);

            std::unique_lock<std::shared_mutex> writeLock(this->cacheMtx);

            File* file = getFilePtr(canonicalUri);

            // Bump up the version
            file->version++;

            // Update content
            file->isValid = isValid;
            file->content.setFullContent(newContent);

            // Update internal state
            file->pendingDeltas.clear();
            file->deltasBaseVersion = file->version;
        }

        // External Management functions (Should not be called within the front-end!)
        // Note: newContent will be reset!
        void VFS::updateFile(const std::string& path, std::string &newContent) {
           std::lock_guard<std::mutex> lock(this->pendingMtx);
            if (this->safetyCount.load(std::memory_order_acquire) > 0) {
                // Driver is running: stage/coalesce the edit without touching `cache` or `index`
                this->pendingRequests[path] = FileChangeRequest{FileChangeRequest::Kind::Full, std::move(newContent)};
                newContent.clear();
                return;
            }
            this->setFileContent(path, newContent);
            newContent.clear();
        }
        void VFS::editFile(const std::string& path, std::vector<ByteEdit> &edits) {
           std::lock_guard<std::mutex> lock(this->pendingMtx);
            if (this->safetyCount.load(std::memory_order_acquire) > 0) {
                if (this->pendingRequests.contains(path)) {
                    // Extend existing edits
                    this->pendingRequests[path].edits.reserve(this->pendingRequests[path].edits.size() + edits.size()); 
                    this->pendingRequests[path].edits.insert(
                        this->pendingRequests[path].edits.end(),
                        std::make_move_iterator(edits.begin()),
                        std::make_move_iterator(edits.end())
                    );
                } else {
                    // Add edits request
                    this->pendingRequests[path] = FileChangeRequest{FileChangeRequest::Kind::Edit, "", std::move(edits)};
                }
                edits.clear();
                return;
            }
            this->editFileContent(path, edits);
            edits.clear();
        }
        void VFS::reloadFile(const std::string& path) {
            std::lock_guard<std::mutex> lock(this->pendingMtx);
            if (this->safetyCount.load(std::memory_order_acquire) > 0) {
                this->pendingRequests[path] = FileChangeRequest{FileChangeRequest::Kind::Reload, ""};
                return;
            }
            this->fetchLocalFileContent(path);
        }
        const std::string VFS::getURI(const FileId& id) {
            std::shared_lock<std::shared_mutex> readLock(this->cacheMtx);

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

            // Allow cleanup when opting out of safe mode
            if (this->safetyCount.fetch_sub(1, std::memory_order_acq_rel) != 1) return;

            // Clean up unneeded deltas
            // Note: Logic that uses this VFS should always run in safe mode!
            // Otherwise, you lose access to the deltas!
            {
                std::unique_lock<std::shared_mutex> writeLock(this->cacheMtx);

                for (File* file : this->dirtyDeltaFiles) {
                    file->pendingDeltas.clear();
                    file->deltasBaseVersion = file->version;
                }
                this->dirtyDeltaFiles.clear();
            }

            // Flush all queued (external) operations!
            for (auto& [path, op] : this->pendingRequests) {
                switch (op.kind) {
                    case FileChangeRequest::Kind::Edit:
                        this->editFileContent(path, op.edits);
                        break;
                    case FileChangeRequest::Kind::Reload:
                        this->fetchLocalFileContent(path);
                        break;
                    case FileChangeRequest::Kind::Full:
                        default:
                        this->setFileContent(path, op.content);
                        break;
                }
            }
            this->pendingRequests.clear();
        }
        bool VFS::isSafeMode() {
            return this->safetyCount.load() > 0;
        }

        // Basic functionality
        const File* VFS::readFile(const std::string& path) {
            const std::string canonicalUri = this->_canonical(path);

            // Fast path: shared lock lookup
            {
                std::shared_lock<std::shared_mutex> readLock(this->cacheMtx);
                auto it = this->index.find(canonicalUri);
                if (it != this->index.end()) {
                    return this->cache[it->second - this->FILE_ID_OFFSET].get();
                }
            }

            // Cache miss: perform disk I/O outside the lock
            std::string newContent;
            const bool isValid = this->_readFile(canonicalUri, newContent);

            // Re-check under exclusive lock (another thread may have inserted it)
            std::unique_lock<std::shared_mutex> writeLock(this->cacheMtx);
            auto it = this->index.find(canonicalUri);
            if (it != this->index.end()) {
                return this->cache[it->second - this->FILE_ID_OFFSET].get();
            }

            // Generate a file object
            File* file = this->addEmptyFile(canonicalUri);

            // Request and load the file!
            // Diagnostic generation is handled by third parties!
            file->isValid = isValid;
            file->content.setFullContent(newContent);

            // Bump up the version
            file->version++;

            // Update internal state
            file->deltasBaseVersion = file->version;

            return file;
        }
        const File* VFS::readFile(const FileId& id) {
            std::shared_lock<std::shared_mutex> readLock(this->cacheMtx);

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
