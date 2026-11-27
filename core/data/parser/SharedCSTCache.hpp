/**
 * @brief
 * Thread-safe LRU CST (least recently used) cache for tree-sitter CST trees
**/

#pragma once

#include "common/headers.hpp"
#include "../host/File.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data {
    namespace Parser {
        using FileId = Data::Host::FileId;
        using FileVersion = Data::Host::FileVersion;
        using CSTHandle = std::shared_ptr<void>;

        struct CSTEntry {
            FileId id = 0;
            FileVersion version = 0;
            CSTHandle tree = nullptr;
        };

        // (Owned by Session, used by Parser)
        class JUG_DATA_API SharedCSTCache {
            public:
                static constexpr size_t DEFAULT_MAX_ENTRIES = 64; // 64 * 32B = 2 KB max container memory

            private:
                // 32 bytes total: 4B id + 4B version + 8B lastUsed + 16B shared_ptr (zero padding)
                struct Slot {
                    FileId id = 0;
                    FileVersion version = 0;
                    uint64_t lastUsed = 0;
                    CSTHandle tree = nullptr;
                };

                mutable std::mutex mtx;
                std::vector<Slot> entries; // Kept sorted by `id`
                uint64_t tick = 0;
                size_t maxEntries = DEFAULT_MAX_ENTRIES;

                std::vector<Slot>::iterator lowerBoundLocked(FileId id) noexcept ;

                // Evicts the least-recently-used slot (or an empty slot first).
                // Moves the evicted tree into `outEvicted` so `ts_tree_delete` runs outside the lock.
                void evictLRULocked(CSTHandle &outEvicted) ;

            public:
                explicit SharedCSTCache(size_t maxCachedEntries = DEFAULT_MAX_ENTRIES)
                    : maxEntries(maxCachedEntries) {}
                ~SharedCSTCache() = default;

                // Non-copyable & non-movable (owns std::mutex)
                SharedCSTCache(const SharedCSTCache&) = delete;
                SharedCSTCache& operator=(const SharedCSTCache&) = delete;
                SharedCSTCache(SharedCSTCache&&) = delete;
                SharedCSTCache& operator=(SharedCSTCache&&) = delete;

                // Fetch cached entry and mark as MRU.
                [[nodiscard]] CSTEntry get(FileId id) ;

                // Atomic lookup for `Parser::updateCST`:
                // - Hit (version == targetVersion): marks MRU, keeps in cache, returns entry.
                // - Stale (version != targetVersion): detaches `tree` in place (keeps slot so the
                //   follow-up `set()` won't shift the vector) and returns exclusive ownership for `oldTree->edit(...)`.
                [[nodiscard]] CSTEntry checkout(FileId id, FileVersion targetVersion) ;

                // Insert or overwrite a file's CST entry (evicts LRU entry if at capacity).
                void set(FileId id, CSTHandle tree, FileVersion version) ;

                // Remove a single file's cached CST.
                void evict(FileId id) ;

                // Clear all cached CSTs (frees trees outside the lock).
                void clear() ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
