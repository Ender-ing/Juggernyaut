/**
 * @brief 
 * "Source code" store
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "types.hpp"
#include "Source.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data {
    namespace Store {
        typedef std::function<void(const SourceId)> EntryCall;
        class JUG_DATA_API SourceStore {
            private:
                std::unordered_map<std::string, SourceId> store; // Uri-based tracking
                std::unordered_map<SourceId, std::unique_ptr<Source>> sources; // ID-based tracking
                std::vector<SourceId> entryPoints;
            public:
                SourceId lastID = 0;
                SourceStore() = default;
                // Fix std::unique_ptr bug
                SourceStore(const SourceStore&) = delete;
                SourceStore& operator=(const SourceStore&) = delete;

                // Unique implementations
                virtual std::string onFileRawRequest(const std::string &uri) = 0;
                // virtual std::string onFileAbsoluteUriRequest(const std::string &uri) = 0;

                // Entry
                void addEntry(SourceId entry) ;
                void removeEntry(SourceId entry) ;
                void resetEntries() ;
                void visitEntries(const EntryCall entryCall) ;

                // Sources
                std::unique_ptr<Source>& getSourceById(const SourceId &id) ;
                void addSource(const std::string &uri, bool isEntry) ;

                // Memory housekeeping
                // ...
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
