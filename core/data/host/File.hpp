/**
 * @brief 
 * File data (for VFS)
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "FIleBuffer.hpp"

namespace Data {
    namespace Host {
        using FileId = uint32_t;
        using FileVersion = uint32_t;
        using Timestamp = uint64_t;

        struct File {
            FileId id = 0;
            std::string canonical = "";
            FileBuffer content; // Can store either valid content, or an error message! (Depends on the value of 'isValid')
            std::vector<EditDelta> pendingDeltas; // Since the last full content reset!
            FileVersion version = 0;
            // bool isLoaded = false;
            bool isValid = false;
            Timestamp lastModified = 0; // Needed for disk-caching in the future!

            // Type-erased CST handle (owned by File, managed by JuggernyautParserLibrary)
            mutable std::shared_ptr<void> cst = nullptr;
            mutable FileVersion cstVersion = 0;
        };
        using FilePtr = std::unique_ptr<File>;
    }
}
