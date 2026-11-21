/**
 * @brief 
 * Virtual File System
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "File.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

// TO-DO: Project index/cache mutations in readFile() using std::shared_mutex!

namespace Data {
    namespace Host {
        // FileBuffer changes
        struct ByteEdit {
            FileByte startByte = 0;
            FileByte oldEndByte = 0;
            std::string text;
        };

        struct FileChangeRequest {
            enum class Kind { Full, Edit, Reload } kind = Kind::Full; // Default to full changes
            std::string content = ""; // Used for Kind::Full
            std::vector<ByteEdit> edits; // Used for Kind::Edits
        };

        class JUG_DATA_API VFS {
            private:
                std::unordered_map<std::string, FileId> index; // Canonical  URI-based tracking
                std::vector<FilePtr> cache; // ID-based cache
                const static uint8_t FILE_ID_OFFSET = 10; // A default file-id offset

                // Multi-threaded safety
                std::mutex pendingMtx;
                std::atomic<uint32_t> safetyCount{0};
                std::unordered_map<std::string, FileChangeRequest> pendingRequests;
                std::vector<File*> dirtyDeltaFiles; // Accumulated deltas tracking (for cleanup)

                // Configs
                bool trackModificationTime = false;

                // Internal
                File* getFilePtr(const std::string& path) ;
                void setFileContent(const std::string& path, std::string &newContent) ;
                void editFileContent(const std::string& path, std::vector<ByteEdit> &edits) ; // Compacts files too!
                void fetchLocalFileContent(const std::string& path) ;

            protected:
                // Local system binds
                virtual bool _readFile(const std::string& path, std::string& output) = 0;
                virtual bool _exists(const std::string& path) = 0;
                virtual bool _isDirectory(const std::string& path) = 0;
                virtual const std::string _canonical(const std::string& path) = 0; // Returns the canonical full path of a file
                virtual const std::string _cwd() = 0;
                virtual Timestamp _lastModified(const std::string& path) = 0;

                // Technical
                File* addEmptyFile(const std::string& canonical) ;
            public:
                VFS() {};
                VFS(bool fileTimestamps) : trackModificationTime(fileTimestamps) {};
                virtual ~VFS() = default;

                // External Management functions (Should not be called within the front-end!)
                // Note: newContent & edits will be moved!
                void updateFile(const std::string& path, std::string& newContent) ; // Kind::Full
                void editFile(const std::string& path, std::vector<ByteEdit> &edits) ; // Kind::Edit
                void reloadFile(const std::string& path) ; // Kind::Reload
                const std::string getURI(const FileId& id) ;

                // Internal Management functions (Only called within the front-end!)
                void setSafeMode(bool safe) ; // Forbids data changes (changes are queued for later)
                bool isSafeMode() ;

                // Basic functionality
                const File* readFile(const FileId& id) ;
                const File* readFile(const std::string& path) ;
                bool exists(const std::string& path) ;
                bool isDirectory(const std::string& path) ;
                const std::string canonical(const std::string& path) ; // Returns the canonical full path of a file
                const std::string cwd() ;

                // Extended functionality
                const std::string getParentDirectory(const std::string& path) ; // e.g. c:\path\to\file.txt -> c:\path\to, c:\path\to -> c:\path,
                const std::string join(const std::string &path, const std::string &append) ;
                bool endsWith(const std::string &path, const std::string &end) ;
                const std::string filename(const std::string &path) ;
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
