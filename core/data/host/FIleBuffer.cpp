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

            FileByte logicalOffset = 0;
            for (const auto& p : this->pieces) { // You must read valid ranges only!
                std::string_view view(this->buffer.data() + p.offset, p.length); // Read the piece's content
                for (size_t i = 0; i < view.size(); ++i) {
                    if (view[i] == '\n') { // Check for new lines
                        this->lineStarts.push_back(logicalOffset + static_cast<FileByte>(i) + 1);
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
            this->totalBytes = static_cast<FileByte>(buffer.size());

            if (this->totalBytes > 0) {
                this->pieces.push_back({0, this->totalBytes});
            }
            this->rebuildLineIndex();
        }
        void FileBuffer::setFullContent(std::string&& content) {
            this->setFullContent(content);
        }

        // Raw data interactions
        TextPoint FileBuffer::pointAtByte(FileByte byteOffset) const noexcept {
            if (byteOffset >= this->totalBytes) {
                FileByte lastLine = static_cast<FileByte>(this->lineStarts.size() - 1);
                return {lastLine, this->totalBytes - this->lineStarts[lastLine]}; // Default to the last line
            }

            auto it = std::upper_bound(this->lineStarts.begin(), this->lineStarts.end(), byteOffset);
            FileByte line = static_cast<FileByte>(std::distance(this->lineStarts.begin(), it) - 1);
            return {line, byteOffset - this->lineStarts[line]};
        }
        std::string_view FileBuffer::getChunk(FileByte byteIndex) const {
            if (byteIndex >= this->totalBytes) {
                return {};
            }

            FileByte curr = 0;
            for (const auto& p : this->pieces) {
                // Check if byteIndex falls inside [curr, curr + p.length)
                if (byteIndex < curr + p.length) {
                    FileByte innerOffset = byteIndex - curr; // piece-relative offset

                    // Return the piece's content, starting from byteIndex!
                    return std::string_view(
                        this->buffer.data() + p.offset + innerOffset,
                        p.length - innerOffset
                    );
                }
                curr += p.length;
            }

            return {};
        }
        EditDelta FileBuffer::applyEdit(FileByte startByte, FileByte oldEndByte, const std::string &insert) { // Don't touch this pls TT
            // Make sure edits remain within [0, this->totalBytes]
            startByte = std::min(startByte, this->totalBytes);
            oldEndByte = std::clamp(oldEndByte, startByte, this->totalBytes);

            // Get line & column info
            TextPoint startPoint = pointAtByte(startByte);
            TextPoint oldEndPoint = pointAtByte(oldEndByte);

            // Calculate newEndByte
            FileByte insertLen = static_cast<FileByte>(insert.size());
            FileByte newEndByte = startByte + insertLen;

            // Compute newEndPoint and collect line breaks within insert
            TextPoint newEndPoint = startPoint;
            std::vector<FileByte> insertLineStarts;
            for (FileByte i = 0; i < insertLen; ++i) {
                if (insert[i] == '\n') {
                    newEndPoint.line++;
                    newEndPoint.column = 0;
                    insertLineStarts.push_back(startByte + i + 1);
                } else {
                    newEndPoint.column++;
                }
            }

            // Append inserted bytes to the buffer
            FileByte appendOffset = static_cast<FileByte>(buffer.size());
            buffer.append(insert);
            Piece newPiece{appendOffset, insertLen};

            // Split and splice piece descriptors
            std::vector<Piece> nextPieces;
            nextPieces.reserve(this->pieces.size() + 2);

            FileByte curr = 0;
            bool isInserted = false;

            auto pushNewPiece = [&]() {
                if (isInserted) {
                    return;
                }
                isInserted = true;
                if (newPiece.length == 0) {
                    return;
                }

                // Coalesce contiguous appends into the trailing piece
                if (!nextPieces.empty() &&
                    nextPieces.back().offset + nextPieces.back().length == newPiece.offset) {
                    nextPieces.back().length += newPiece.length;
                } else {
                    nextPieces.push_back(newPiece);
                }
            };
            for (const auto& p : this->pieces) {
                uint32_t pStart = curr;
                uint32_t pEnd = curr + p.length;

                if (pEnd <= startByte) {
                    nextPieces.push_back(p);
                } else if (pStart >= oldEndByte) {
                    pushNewPiece();
                    nextPieces.push_back(p);
                } else {
                    if (pStart < startByte) {
                        nextPieces.push_back({p.offset, startByte - pStart});
                    }
                    pushNewPiece();
                    if (pEnd > oldEndByte) {
                        uint32_t skip = oldEndByte - pStart;
                        nextPieces.push_back({p.offset + skip, pEnd - oldEndByte});
                    }
                }
                curr = pEnd;
            }
            pushNewPiece();

            // Replace pieces by a cleaner version
            this->pieces = std::move(nextPieces);
            int64_t byteDelta = static_cast<int64_t>(newEndByte) - static_cast<int64_t>(oldEndByte);
            this->totalBytes = static_cast<uint32_t>(static_cast<int64_t>(this->totalBytes) + byteDelta);

            // Patch lineStarts table
            // Remove line starts that fell inside the deleted range
            auto eraseBegin = this->lineStarts.begin() + startPoint.line + 1;
            auto eraseEnd = this->lineStarts.begin() + oldEndPoint.line + 1;
            auto insertPos = this->lineStarts.erase(eraseBegin, eraseEnd);
            // Splice in new line starts collected from `inserted` in Step 1
            insertPos = this->lineStarts.insert(
                insertPos,
                insertLineStarts.begin(),
                insertLineStarts.end()
            );

            // Shift every surviving line after the edit by `byte_delta`
            for (auto it = insertPos + insertLineStarts.size(); it != lineStarts.end(); ++it) {
                *it = static_cast<uint32_t>(static_cast<int64_t>(*it) + byteDelta);
            }

            return EditDelta{
                startByte,
                oldEndByte,
                newEndByte,
                startPoint,
                oldEndPoint,
                newEndPoint
            };
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
        FileByte FileBuffer::byteLength() const noexcept {
            return this->totalBytes;
        }
        FileByte FileBuffer::lineCount() const noexcept {
            return static_cast<FileByte>(this->lineStarts.size());
        }
        FileByte FileBuffer::lineStartByte(FileByte line) const noexcept {
            if (line >= this->lineStarts.size()) {
                return this->totalBytes; // Default to the end of the file
            }
            return this->lineStarts[line];
        }
    }
}
