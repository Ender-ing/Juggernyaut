/**
 * @brief 
 * "Source code" store
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_STORE_API

#include "Source.hpp"

// Basic C++ headers
#include <unordered_map>

namespace Store {
    class JUG_STORE_API SourceStore {
        private:
            std::unordered_map<std::string, Source> sources;
        public:
            // ...
            SourceStore() = default;
    };
}
