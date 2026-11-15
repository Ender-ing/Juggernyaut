/**
 * @brief 
 * File buffers (for VFS)
**/

#include "FIleBuffer.hpp"

namespace Data {
    namespace Host {
        void FileBuffer::setFullContent(std::string &newData) {
            data = std::move(newData);
        }
        void FileBuffer::applyEdit(FileByte start, FileByte oldEnd, const std::string &insert) {
            // ...
        }
        std::string_view FileBuffer::getChunk(ByteIndex index) const {
            if (index >= data.size()) {
                return "";
            }

            return std::string_view(data).substr(index);
        }
    }
}
