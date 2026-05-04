/**
 * @brief
 * Manage file printing
**/

// Basic C++
#include <fstream>
#include <string_view>
#include <algorithm>

#include "source.hpp"

namespace Console {
    namespace Source {
        namespace Internal {
            std::string getRawFile(const std::string& path) {
                std::ifstream file(path, std::ios::in | std::ios::binary);
                if (!file) return "";

                // Jump to the end to get file size
                file.seekg(0, std::ios::end);
                std::string content;
                auto fileSize = file.tellg();

                if (fileSize <= 0) return "";
                content.resize(static_cast<size_t>(file.tellg()));
    
                // Jump back and read everything in one single system call
                file.seekg(0, std::ios::beg);
                file.read(content.data(), static_cast<std::streamsize>(content.size()));
    
                return content;
            }
            Offsets getByteOffset(std::string_view content, size_t startLine, size_t endLine, bool &paddedStart) {
                Offsets offsets;

                size_t offset = 0;

                if (startLine != 0) {
                    paddedStart = true;
                    startLine -= 1;
                }
                endLine += 1;
    
                // Fast-forward through newlines
                for (size_t i = 0; i < startLine; ++i) {
                    offset = content.find('\n', offset);
                    if (offset == std::string_view::npos) {
                        offset = content.size(); // Line out of bounds, return EOF
                        break;
                    }
                    offset++; // Skip '\n'
                }
                offsets.start = offset;

                // Fast-forward through the rest of the newlines
                for (size_t i = startLine; i <= endLine; ++i) {
                    offset = content.find('\n', offset);
                    if (offset == std::string_view::npos) {
                        offset = content.size(); // Line out of bounds, return EOF
                        break;
                    }
                    offset++; // Skip '\n'
                }
                offsets.end = offset;

                return offsets;
            }

        }
        std::string extractRange(const std::string& path, size_t startLine, size_t endLine, bool &paddedStart) {
            std::string content = Internal::getRawFile(path);
            if (content.empty()) return "";

            std::string_view view = content;

            Offsets offsets = Internal::getByteOffset(view, startLine, endLine, paddedStart);

            if (offsets.start > offsets.end) return "<INVALID_SOURCE_RANGE>";

            // Substr on a string_view doesn't copy! We only allocate a std::string ONCE at the end.
            return std::string(view.substr(offsets.start, offsets.end - offsets.start));
        }
    }
}
