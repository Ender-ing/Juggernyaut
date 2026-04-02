/**
 * @brief 
 * "Source code" store
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "Source.hpp"

// Basic C++ headers
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types

namespace Data {
    namespace Store {
        class Source;
        class JUG_DATA_API SourceStore {
            private:
                std::unordered_map<std::string, Source> sources;
            public:
                SourceStore() = default;

                // Unique implementations
                virtual std::string onFileRawRequest(const std::string &uri) = 0;
        };
    }
}

#pragma warning(pop)
