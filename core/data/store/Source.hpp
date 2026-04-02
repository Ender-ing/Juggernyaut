/**
 * @brief 
 * "Source code" instance
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "SourceStore.hpp"

#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types

namespace Data {
    namespace Store {
        class SourceStore;
        class JUG_DATA_API Source {
            private:
                std::string rawContent;
                std::string uri;
                SourceStore *store;
                void fetchRawContent() ;

                // Tracking
                bool shouldUpdateRawContent = true;
            public:
                Source() = default;
                Source(SourceStore *srcStore, std::string srcUri)
                : store(srcStore), uri(srcUri) {}
            
                // Tracking
                void requestRawUpdate() ;

                // You know nothing about the source's raw content!
                // This is handled by the store object!
                const std::string& getRawContent() ;
        };
    }
}

#pragma warning(pop)
