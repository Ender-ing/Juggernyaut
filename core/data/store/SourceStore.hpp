/**
 * @brief 
 * "Source code" store
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "types.hpp"
#include "Source.hpp"

// Basic C++ headers
#include <functional>
#include <unordered_map>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Data {
    namespace Store {
        typedef std::function<void(const SourceID)> EntryCall;
        class JUG_DATA_API SourceStore {
            private:
                std::unordered_map<std::string, SourceID> store; // Uri-based tracking
                std::unordered_map<SourceID, std::unique_ptr<Source>> sources; // ID-based tracking
                std::vector<SourceID> entryPoints;
            public:
                SourceID lastID = 0;
                SourceStore() = default;
                // Fix std::unique_ptr bug
                SourceStore(const SourceStore&) = delete;
                SourceStore& operator=(const SourceStore&) = delete;

                // Unique implementations
                virtual std::string onFileRawRequest(const std::string &uri) = 0;
                // virtual std::string onFileAbsoluteUriRequest(const std::string &uri) = 0;

                // Entry
                void addEntry(SourceID entry) ;
                void removeEntry(SourceID entry) ;
                void resetEntries() ;
                void visitEntries(const EntryCall entryCall) ;

                // Sources
                std::unique_ptr<Source>& getSourceById(const SourceID &id) ;
                void addSource(const std::string &uri, bool isEntry) ;

                // Memory housekeeping
                // ...
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
