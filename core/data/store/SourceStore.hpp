/**
 * @brief 
 * "Source code" store
**/

#pragma once

#include "common/headers.hpp"
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
            protected:
                std::unordered_map<std::string, SourceId> index; // Uri-based tracking
            private:
                std::unordered_map<SourceId, std::unique_ptr<Source>> sources; // ID-based tracking
                std::vector<SourceId> entryPoints;

                std::vector<std::string> importDirs;
            public:
                SourceId lastID = 10; // 0-10 are used for internal purposes
                SourceStore() = default;
                // Fix std::unique_ptr bug
                SourceStore(const SourceStore&) = delete;
                SourceStore& operator=(const SourceStore&) = delete;
                virtual ~SourceStore() = default;

                // Unique implementations
                virtual bool _getRawFile(const std::string &uri, std::string &output) = 0;
                virtual bool _isFileAccessible(const std::string &uri) = 0;
                virtual bool _isDirValid(const std::string &path) = 0;
                virtual std::string _getFileExtension(const std::string &uri) = 0;
                virtual std::string _getPathDir(const std::string &uri) = 0;
                virtual std::string _getCanonical(const std::string &uri) = 0;
                virtual std::string _joinPaths(const std::string &base, const std::string &path) = 0;

                // Import paths
                void addImportDir(const std::string &path) ;
                void resetImportDirs() ;

                // General path lookup
                // resolvePath:
                // Returns: <is_success> (bool)
                // Note: <output> is used as an error message container on failure
                bool resolvePath(const std::string &uri, std::string &output, SourceId callerId = 0) ;

                // Entry
                void addEntry(SourceId entry) ;
                void removeEntry(SourceId entry) ;
                void resetEntries() ;
                void visitEntries(const EntryCall entryCall) ;

                // IDs
                SourceId getSourceIdByUri(const std::string &uri) ;

                // Sources
                std::unique_ptr<Source>& getSourceById(const SourceId &id) ;
                std::unique_ptr<Source>* getSourceByUri(const std::string &uri) ;
                void addSource(const std::string &uri, bool isEntry) ;

                // Memory housekeeping
                virtual void deleteSource(std::unique_ptr<Source> &src, bool erase = true) ; // Allow override
                void cleanup() ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
