/**
 * @brief 
 * "Source code" instance
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "types.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data {
    namespace Store {
        class JUG_DATA_API Source {
            public:
            private:
                SourceID id;
                std::string uri;
                SourceStore *store;
                std::string rawContent;
                void fetchRawContent() ;

                // Tracking
                bool isEntryPoint = false;
                bool shouldUpdateRawContent = true;

                // Dependency tracking
                std::vector<SourceID> neededSources;
            public:
                Source() = default;
                Source(std::string srcUri, SourceStore *srcStore) ;

                // Tracking
                SourceID getID();

                // Dependency tracking
                void addSourceDependency(SourceID dep) ;
                void removeSourceDependency(SourceID dep) ;
                void resetSourceDependencies() ;

                // Content Tracking
                void requestRawUpdate() ;

                // You know nothing about the source's raw content!
                // This is handled by the store object!
                const std::string& getRawContent() ;

                // If a file is imported by the user (not through an internal core process),
                // a source should be treated as an entry point!
                void setIsEntryPoint(const bool state) ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
