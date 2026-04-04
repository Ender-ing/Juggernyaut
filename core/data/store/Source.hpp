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
        typedef std::function<void(const SourceId&)> DependencyCall;
        class JUG_DATA_API Source {
            public:
            private:
                SourceId id;
                std::string uri;
                SourceStore *store;
                std::string rawContent;
                void fetchRawContent() ;

                // Tracking
                bool isEntryPoint = false;
                bool shouldUpdateRawContent = true;
                bool shouldUpdateAST = true;

                // Dependency tracking
                std::vector<SourceId> neededSources;
            public:
                Source(std::string srcUri, SourceStore *srcStore) ;

                // Tracking
                SourceId getId();

                // Dependency tracking
                void addSourceDependency(SourceId dep) ;
                void removeSourceDependency(SourceId dep) ;
                void resetSourceDependencies() ;
                void visitDependencies(DependencyCall depCall) ;

                // Content Tracking
                void requestRawUpdate() ;

                // You know nothing about the source's raw content!
                // This is handled by the store object!
                const std::string& getRawContent() ;

                //  Parser: AST
                void setUpdateAST(const bool state) ;
                const bool getUpdateAST() ;

                // If a file is imported by the user (not through an internal core process),
                // a source should be treated as an entry point!
                void setIsEntryPoint(const bool state) ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
