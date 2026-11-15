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
            uint32_t line = 0;   // 0-based line index
            uint32_t column = 0; // 0-based UTF-8 byte offset from start of line
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
                    uint32_t offset; // relative to the start of the buffer
                    uint32_t length; // edit's content length
                };

                std::string buffer;
                std::vector<Piece> pieces;
                std::vector<uint32_t> lineStarts{0};
                uint32_t totalBytes = 0;

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
                TextPoint pointAtByte(uint32_t byte_offset) const noexcept ;
                std::string_view getChunk(FileByte byteIndex) const ;
                void applyEdit(FileByte startByte, FileByte oldEndByte, const std::string &insert) ;

                // Technical
                std::string toString() const;
                void compact() ; // Cleans up the internal state!

                // General queries
                uint32_t byteLength() const noexcept ;
                uint32_t lineCount() const noexcept ;
                uint32_t lineStartByte(uint32_t line) const noexcept ;
        };
    }
}
