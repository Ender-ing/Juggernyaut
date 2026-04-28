/**
 * @brief 
 * "Source code" instance
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "../../diagnostics/Diagnostic.hpp"

#include "types.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data {
    namespace Store {
        typedef std::function<void(const SourceId&)> DependencyCall;
        typedef std::function<void(const Diagnostics::Diagnostic&)> DiagnosticCall;

        struct DiagnosticsSection {
            std::vector<Diagnostics::Diagnostic> diagnostics;

            // Statistics
            uint32_t errors = 0;
            uint32_t warnings = 0;
            uint32_t info = 0;
            uint32_t hints = 0;
        };
        struct DiagnosticsBatch {
            DiagnosticsSection parser;
        };

        class JUG_DATA_API Source {
            public:
            private:
                SourceId id;
                SourceStore *store;
                std::string rawContent;
                void fetchRawContent() ;

                // Tracking
                bool isEntryPoint = false;
                bool shouldUpdateRawContent = true;
                bool shouldUpdateAST = true;

                // Dependency tracking
                std::vector<SourceId> neededBy;
                std::vector<SourceId> neededSources;

                // Diagnostics
                DiagnosticsBatch diagnostics;
            public:
                const std::string uri;
                uint32_t round = 0;

                Source(std::string srcUri, SourceStore *srcStore) ;
                Source(const Source&) = delete;
                Source& operator=(const Source&) = delete;
                virtual ~Source() = default;

                // Tracking
                SourceId getId();

                // Dependency tracking
                void addSourceDependency(SourceId dep) ;
                void removeSourceDependency(SourceId dep) ;
                void resetSourceDependencies() ;
                void visitDependencies(DependencyCall depCall) ;

                // Content Tracking
                void invalidateRawContent() ;
                void invalidateAST() ;
                const bool getUpdateAST() ;
                void preventUpdateAST() ;

                // You know nothing about the source's raw content!
                // This is handled by the store object!
                const std::string& getRawContent() ;

                // If a file is imported by the user (not through an internal core process),
                // a source should be treated as an entry point!
                void setIsEntryPoint(const bool state) ;

                // Diagnostics
                void addParserDiagnostic(Diagnostics::Diagnostic diag) ;
                void resetParserDiagnostics() ;
                void visitParserDiagnostics(DiagnosticCall) ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
