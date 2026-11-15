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
        using ByteIndex = uint32_t;

        class JUG_DATA_API FileBuffer {
            private:
                std::string data;
            public:
                FileBuffer() {};
                virtual ~FileBuffer() = default;

                void setFullContent(std::string &newData) ;
                void applyEdit(FileByte start, FileByte oldEnd, const std::string &insert) ;
                std::string_view getChunk(ByteIndex index) const ;
        };
    }
}
