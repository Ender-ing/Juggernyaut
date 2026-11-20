/**
 * @brief
 * Thread-safe LRU CST (least recently used) cache for tree-sitter CST trees
**/


#include "SharedCSTCache.hpp"

namespace Data {
    namespace Parser {
        std::vector<SharedCSTCache::Slot>::iterator SharedCSTCache::lowerBoundLocked(FileId id) noexcept {
            return std::lower_bound( // Binary seach
                this->entries.begin(),
                this->entries.end(),
                id,
                [](const Slot &slot, FileId targetId) noexcept {
                    return slot.id < targetId;
                }
            );
        }

        // Evicts the least-recently-used slot (or an empty slot first).
        // Moves the evicted tree into `outEvicted` so `ts_tree_delete` runs outside the lock.
        void SharedCSTCache::evictLRULocked(CSTHandle &outEvicted) {
            if (this->entries.empty()) {
                return;
            }

            auto victim = this->entries.begin();
            for (auto it = this->entries.begin(); it != this->entries.end(); ++it) {
                if (it->tree == nullptr) {
                    victim = it; // Prefer reclaiming an empty checked-out slot
                    break;
                }
                if (victim->tree != nullptr && it->lastUsed < victim->lastUsed) {
                    victim = it;
                }
            }

            outEvicted = std::move(victim->tree);
            this->entries.erase(victim);
        }

        // Fetch cached entry and mark as MRU.
        [[nodiscard]] CSTEntry SharedCSTCache::get(FileId id) {
            std::lock_guard<std::mutex> lock(this->mtx);
            auto it = this->lowerBoundLocked(id);
            if (it != this->entries.end() && it->id == id && it->tree != nullptr) {
                it->lastUsed = ++(this->tick);
                return CSTEntry{
                    .id = it->id,
                    .version = it->version,
                    .tree = it->tree
                };
            }
            return {};
        }

        // Atomic lookup for `Parser::updateCST`:
        // - Hit (version == targetVersion): marks MRU, keeps in cache, returns entry.
        // - Stale (version != targetVersion): detaches `tree` in place (keeps slot so the
        //   follow-up `set()` won't shift the vector) and returns exclusive ownership for `oldTree->edit(...)`.
        [[nodiscard]] CSTEntry SharedCSTCache::checkout(FileId id, FileVersion targetVersion) {
            std::lock_guard<std::mutex> lock(this->mtx);
            auto it = this->lowerBoundLocked(id);
            if (it == this->entries.end() || it->id != id || it->tree == nullptr) {
                return {};
            }

            it->lastUsed = ++(this->tick);
            if (it->version == targetVersion) {
                return CSTEntry{
                    .id = it->id,
                    .version = it->version,
                    .tree = it->tree
                };
            }

            return CSTEntry{
                .id = it->id,
                .version = it->version,
                .tree = std::move(it->tree) // This is an outdated tree!
            };
        }

        // Insert or overwrite a file's CST entry (evicts LRU entry if at capacity).
        void SharedCSTCache::set(FileId id, CSTHandle tree, FileVersion version) {
            if (tree == nullptr) {
                this->evict(id);
                return;
            }

            CSTHandle evictedOrReplaced = nullptr;
            {
                std::lock_guard<std::mutex> lock(this->mtx);
                if (this->maxEntries == 0) {
                    evictedOrReplaced = std::move(tree);
                    return;
                }

                auto it = this->lowerBoundLocked(id);

                // Fast in-place update if slot already exists (common path after `checkout()`)
                if (it != this->entries.end() && it->id == id) {
                    evictedOrReplaced = std::move(it->tree);
                    it->tree = std::move(tree);
                    it->version = version;
                    it->lastUsed = ++this->tick;
                    return;
                }

                // At capacity: evict LRU entry before inserting new slot
                if (this->entries.size() >= this->maxEntries) {
                    this->evictLRULocked(evictedOrReplaced);
                    it = this->lowerBoundLocked(id);
                }

                this->entries.insert(it, Slot{
                    .id = id,
                    .version = version,
                    .lastUsed = ++this->tick,
                    .tree = std::move(tree)
                });
            }
            // `evictedOrReplaced` destructs outside `mtx`
        }

        // Remove a single file's cached CST.
        void SharedCSTCache::evict(FileId id) {
            CSTHandle dropped = nullptr;
            {
                std::lock_guard<std::mutex> lock(this->mtx);
                auto it = this->lowerBoundLocked(id);
                if (it != this->entries.end() && it->id == id) {
                    dropped = std::move(it->tree);
                    this->entries.erase(it);
                }
            }
        }

        // Clear all cached CSTs (frees trees outside the lock).
        void SharedCSTCache::clear() {
            std::vector<Slot> drained;
            {
                std::lock_guard<std::mutex> lock(this->mtx);
                drained.swap(this->entries);
                this->tick = 0;
            }
        }
    }
}
