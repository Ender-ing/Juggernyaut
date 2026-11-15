/**
 * @brief 
 * File buffers (for VFS)
**/

#include "FIleBuffer.hpp"

// C++ headers
#include <algorithm>
#include <iterator>

namespace Data {
    namespace Host {
        // Internal
        void FileBuffer::rebuildLineIndex() {
            this->lineStarts.clear();
            this->lineStarts.push_back(0);

            uint32_t logicalOffset = 0;
            for (const auto& p : this->pieces) { // You must read valid ranges only!
                std::string_view view(this->buffer.data() + p.offset, p.length); // Read the piece's content
                for (size_t i = 0; i < view.size(); ++i) {
                    if (view[i] == '\n') { // Check for new lines
                        this->lineStarts.push_back(logicalOffset + static_cast<uint32_t>(i) + 1);
                    }
                }
                logicalOffset += p.length;
            }
        }

        // File's base (resets internal buffer state)
        void FileBuffer::setFullContent(std::string &content) {
            this->buffer = std::move(content);
            content.clear();
            this->pieces.clear();
            this->totalBytes = static_cast<uint32_t>(buffer.size());

            if (this->totalBytes > 0) {
                this->pieces.push_back({0, this->totalBytes});
            }
            this->rebuildLineIndex();
        }
        void FileBuffer::setFullContent(std::string&& content) {
            this->setFullContent(content);
        }

        // Raw data interactions
        TextPoint FileBuffer::pointAtByte(uint32_t byteOffset) const noexcept {
            if (byteOffset >= this->totalBytes) {
                uint32_t lastLine = static_cast<uint32_t>(this->lineStarts.size() - 1);
                return {lastLine, this->totalBytes - this->lineStarts[lastLine]}; // Default to the last line
            }

            auto it = std::upper_bound(this->lineStarts.begin(), this->lineStarts.end(), byteOffset);
            uint32_t line = static_cast<uint32_t>(std::distance(this->lineStarts.begin(), it) - 1);
            return {line, byteOffset - this->lineStarts[line]};
        }
        std::string_view FileBuffer::getChunk(FileByte byteIndex) const {
            if (byteIndex >= this->totalBytes) {
                return {};
            }

            // [!]

            uint32_t curr = 0;
            for (const auto& p : this->pieces) {
                if (byteIndex < curr + p.length) {
                    uint32_t innerOffset = byteIndex - curr;
                    return std::string_view(
                        this->buffer.data() + p.offset + innerOffset,
                        p.length - innerOffset
                    );
                }
                curr += p.length;
            }

            return {};
        }
        void FileBuffer::applyEdit(FileByte startByte, FileByte oldEndByte, const std::string &insert) {
            // ...
        }

        // Technical
        std::string FileBuffer::toString() const {
            std::string out;
            out.reserve(this->totalBytes);

            // Copy the actual file's raw data
            for (const auto& p : this->pieces) {
                out.append(this->buffer.data() + p.offset, p.length);
            }
            return out;
        }
        void FileBuffer::compact() {
            if (this->pieces.size() <= 1 && this->buffer.size() == this->totalBytes) {
                return;
            }

            std::string compacted = this->toString();
            this->buffer = std::move(compacted);
            this->pieces.clear();
            if (this->totalBytes > 0) {
                this->pieces.push_back({0, this->totalBytes});
            }
        }

        // General queries
        uint32_t FileBuffer::byteLength() const noexcept {
            return this->totalBytes;
        }
        uint32_t FileBuffer::lineCount() const noexcept {
            return static_cast<uint32_t>(this->lineStarts.size());
        }
        uint32_t FileBuffer::lineStartByte(uint32_t line) const noexcept {
            if (line >= this->lineStarts.size()) {
                return this->totalBytes; // Default to the end of the file
            }
            return this->lineStarts[line];
        }
    }
}



/*
        // Pure byte-range delta mutation
        EditDelta applyEdit(uint32_t start_byte, uint32_t old_end_byte, std::string_view inserted) {
            start_byte = std::min(start_byte, total_bytes);
            old_end_byte = std::clamp(old_end_byte, start_byte, total_bytes);

            TextPoint start_point = pointAtByte(start_byte);
            TextPoint old_end_point = pointAtByte(old_end_byte);

            uint32_t inserted_len = static_cast<uint32_t>(inserted.size());
            uint32_t new_end_byte = start_byte + inserted_len;

            // 1. Compute new_end_point and collect line breaks within inserted text
            TextPoint new_end_point = start_point;
            std::vector<uint32_t> inserted_line_starts;
            for (uint32_t i = 0; i < inserted_len; ++i) {
                if (inserted[i] == '\n') {
                    new_end_point.line++;
                    new_end_point.column = 0;
                    inserted_line_starts.push_back(start_byte + i + 1);
                } else {
                    new_end_point.column++;
                }
            }

            // 2. Append inserted bytes to the single append-only buffer
            uint32_t append_offset = static_cast<uint32_t>(buffer.size());
            buffer.append(inserted);
            Piece new_piece{append_offset, inserted_len};

            // 3. Split and splice piece descriptors
            std::vector<Piece> next_pieces;
            next_pieces.reserve(pieces.size() + 2);

            uint32_t curr = 0;
            bool is_inserted = false;

            auto push_new_piece = [&]() {
                if (is_inserted) return;
                is_inserted = true;
                if (new_piece.length == 0) return;

                // Coalesce contiguous appends into the trailing piece
                if (!next_pieces.empty() &&
                    next_pieces.back().offset + next_pieces.back().length == new_piece.offset) {
                    next_pieces.back().length += new_piece.length;
                } else {
                    next_pieces.push_back(new_piece);
                }
            };

            for (const auto& p : pieces) {
                uint32_t p_start = curr;
                uint32_t p_end = curr + p.length;

                if (p_end <= start_byte) {
                    next_pieces.push_back(p);
                } else if (p_start >= old_end_byte) {
                    push_new_piece();
                    next_pieces.push_back(p);
                } else {
                    if (p_start < start_byte) {
                        next_pieces.push_back({p.offset, start_byte - p_start});
                    }
                    push_new_piece();
                    if (p_end > old_end_byte) {
                        uint32_t skip = old_end_byte - p_start;
                        next_pieces.push_back({p.offset + skip, p_end - old_end_byte});
                    }
                }
                curr = p_end;
            }
            push_new_piece();

            pieces = std::move(next_pieces);
            int64_t byte_delta = static_cast<int64_t>(new_end_byte) - static_cast<int64_t>(old_end_byte);
            total_bytes = static_cast<uint32_t>(static_cast<int64_t>(total_bytes) + byte_delta);

            // 4. Patch line_starts table
            auto erase_begin = line_starts.begin() + start_point.line + 1;
            auto erase_end = line_starts.begin() + old_end_point.line + 1;
            auto insert_pos = line_starts.erase(erase_begin, erase_end);
            insert_pos = line_starts.insert(
                insert_pos,
                inserted_line_starts.begin(),
                inserted_line_starts.end()
            );

            for (auto it = insert_pos + inserted_line_starts.size(); it != line_starts.end(); ++it) {
                *it = static_cast<uint32_t>(static_cast<int64_t>(*it) + byte_delta);
            }

            return EditDelta{
                start_byte,
                old_end_byte,
                new_end_byte,
                start_point,
                old_end_point,
                new_end_point
            };
        }
    };

*/