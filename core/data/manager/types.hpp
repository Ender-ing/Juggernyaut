/**
 * @brief 
 * Manager communication
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "../store/types.hpp"

namespace Data {
    namespace Manager {
        using LibraryPath = std::vector<std::string>;

        // Sessions are blind to the manager's actions!
        // [ResolveLibraryPath]
        // Returns: Symbol table reference from the required module's source.
        typedef std::function<const void*(const LibraryPath&)> ResolveLibraryPath;
    }
}
