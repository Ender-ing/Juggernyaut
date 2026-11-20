/**
 * @brief 
 * File buffers (for VFS)
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

// TO-DO: Project index/cache mutations in readFile() using std::shared_mutex!

namespace Data {
    namespace Host {
        using FileByte = uint32_t;

        struct TextPoint {
            FileByte line = 0;   // 0-based line index
            FileByte column = 0; // 0-based UTF-8 byte offset from start of line
        };

        struct EditDelta {
            // Offset info
            FileByte startByte = 0;
            FileByte oldEndByte = 0;
            FileByte newEndByte = 0;

            // In-file point info
            TextPoint startPoint;
            TextPoint oldEndPoint;
            TextPoint newEndPoint;
        };

        class JUG_DATA_API FileBuffer {
            private:
                // New buffer edit location info
                struct Piece {
                    FileByte offset; // relative to the start of the buffer
                    FileByte length; // edit's content length
                };

                std::string buffer;
                std::vector<Piece> pieces;
                std::vector<FileByte> lineStarts{0};
                FileByte totalBytes = 0;

                std::string data; // REMOVE LATER

                // Internal
                void rebuildLineIndex() ;

            public:
                FileBuffer() = default;
                virtual ~FileBuffer() = default;

                // File's base (resets internal buffer state)
                void setFullContent(std::string &content) ;
                void setFullContent(std::string &&content) ;

                // Raw data interactions
                TextPoint pointAtByte(FileByte byteOffset) const noexcept ;
                std::string_view getChunk(FileByte byteIndex) const ;
                EditDelta applyEdit(FileByte startByte, FileByte oldEndByte, const std::string &insert) ;

                // Technical
                std::string_view view() const; // Used by VFS's clients in safe mode!
                std::string toString() const;
                void compact() ; // Cleans up the internal state!

                // General queries
                FileByte byteLength() const noexcept ;
                FileByte lineCount() const noexcept ;
                FileByte lineStartByte(FileByte line) const noexcept ;
        };
    }
}
