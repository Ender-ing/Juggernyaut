/**
 * @brief
 * Handle CLI report displays
**/

#include "include/console/report.hpp"

// Comms headers
#include "include/console/console.hpp"
#include "include/console/basic.hpp"
#include "include/console/source.hpp"

// Shortent the syntax for printing to the console
#define INTERNAL_C_OUT(DATA, CHANNEL_VAR)                   \
    ((CHANNEL_VAR == 0 ? std::cout : std::clog) << DATA)

namespace Console {
    namespace Internal {
        // Handle reports CLI outputs
        namespace Reports {
            // Sanitise strings for console output
            std::string sanitize(const std::string &str) {
                std::string result;
                result.reserve(str.length()); 

                for (char c : str) {
                    if (c == '{') {
                        result += "{{";
                    } else if (c == '}') {
                        result += "}}";
                    } else {
                        result += c;
                    }
                }

                return result;
            }

            // Padding
            void padLeft(std::string& str, size_t min) {
                if (str.size() < min) {
                    // insert(index, count, character)
                    str.insert(0, min - str.size(), '-'); 
                }
            }

            // Keep track of print statistics
            static bool isFirstPrint = true;

            // Print report details
            int lastChannel = 0;
            void print() {
                // Track output data printing
                uint32_t color;
                int channel = 0; // [0 -> cout, 1 -> clog]
                bool shouldColor = true;
                bool shouldPrompt = true;
                std::stringstream prompt;
                auto out = [&channel, &color, &shouldColor](const std::string &data) {
                    // {fmt}
                    const std::string &final = sanitize(data);

                    if (lastChannel != channel) {
                        INTERNAL_C_OUT(std::flush, lastChannel);
                        lastChannel = channel;
                    }

                    // Print to the chosen output channel
                    if (shouldColor) {
                        INTERNAL_C_OUT(Console::Internal::color(final, color), channel);
                    } else {
                        INTERNAL_C_OUT(final, channel);
                    }
                };

                switch (IndividualReport::type) {
                case WARNING_REPORT:
                    color = Color::golden_rod;
                    channel = 0;
                    // Title
                    if (!IndividualReport::isContinuation) {
                        prompt << "[Warning]";
                    }
                    break;
                case CRITICAL_REPORT:
                    color = Color::crimson;
                    channel = 1;
                    // Title
                    if (!IndividualReport::isContinuation) {
                        prompt << "[Error]";
                    }
                    break;
                case FATAL_REPORT:
                    color = Color::crimson;
                    channel = 1;
                    // Title
                    if (!IndividualReport::isContinuation) {
                        prompt << "[Fatal Error]";
                    }
                    break;
                case ACTION_REPORT:
                    color = Color::sea_green;
                    channel = 0;
                    // Title
                    if (!IndividualReport::isContinuation) {
                        prompt << "[Action]";
                    }
                    break;
                case DEBUG_REPORT:
                    color = Color::blue_violet;
                    channel = 0;
                    // Title
                    if (!IndividualReport::isContinuation) {
                        prompt << "[Debug]";
                    }
                    break;
                default:
                    shouldColor = false;
                    channel = 0;
                    // No prompts
                    shouldPrompt = false;
                }

                // Print a new line for new reports
                if (!isFirstPrint) {
                    INTERNAL_C_OUT('\n', channel);
                }
                
                // Print report type info
                if (shouldPrompt) {
                    if (IndividualReport::stage.length() > 0) {
                        prompt << " (" << IndividualReport::stage << ")";
                    }
                    if (IndividualReport::code != 1) {
                        prompt << " #" << Console::IndividualReport::code << " ";
                    } else {
                        prompt << " ";
                    }
                    out(prompt.str());
                }

                // Print report
                out(IndividualReport::messageStream.str());

                // Print source
                if (IndividualReport::path != "") {
                    // Reset stream
                    IndividualReport::messageStream.str(""); // Clear the internal buffer
                    IndividualReport::messageStream.clear(); // Clear the state flags (eofbit, failbit, badbit)

                    bool paddedStart = false;
                    std::string rawFrame = Source::extractRange(IndividualReport::path, IndividualReport::startLine, IndividualReport::endLine, paddedStart);
                    auto colorSrc = [&shouldColor](const std::string &str, const uint32_t clr) {
                        if (shouldColor) {
                            const std::string &final = sanitize(str);
                            return Console::Internal::color(final, clr);
                        } else {
                            return str;
                        }
                    };

                    IndividualReport::messageStream << "\n" << colorSrc("From ", Console::Internal::Color::gray)
                        << colorSrc(IndividualReport::path, Console::Internal::Color::gray)
                        << colorSrc("(", Console::Internal::Color::gray)
                        << colorSrc(std::to_string(IndividualReport::startLine + 1), Console::Internal::Color::gray)
                        << colorSrc(":", Console::Internal::Color::gray)
                        << colorSrc(std::to_string(IndividualReport::startColumn + 1), Console::Internal::Color::gray)
                        << colorSrc(")", Console::Internal::Color::gray);

                    if (IndividualReport::startLine != IndividualReport::endLine || IndividualReport::startColumn != IndividualReport::endColumn) {
                        IndividualReport::messageStream << colorSrc(":", Console::Internal::Color::gray);

                        std::string_view frameView = rawFrame;

                        // Start offset
                        size_t startOffset = 0;
                        if (paddedStart) {
                            startOffset = frameView.find('\n', startOffset);
                            if (startOffset != std::string_view::npos) startOffset++; // Skip '\n'
                        }
                        startOffset = std::min(startOffset + IndividualReport::startColumn, frameView.size());

                        // End offset
                        size_t endOffset = startOffset;
                        size_t linesInMatch = IndividualReport::endLine - IndividualReport::startLine;
    
                        for (size_t i = 0; i < linesInMatch; ++i) {
                            endOffset = frameView.find('\n', endOffset);
                            if (endOffset != std::string_view::npos) endOffset++; // Skip the '\n'
                        }
                        if (linesInMatch == 0) {
                            endOffset = startOffset + (IndividualReport::endColumn - IndividualReport::startColumn);
                        } else {
                            endOffset += IndividualReport::endColumn; 
                        }
                        endOffset = std::min(endOffset, frameView.size());

                        std::string_view preRange  = frameView.substr(0, startOffset);
                        std::string_view range = frameView.substr(startOffset, endOffset - startOffset);
                        std::string_view postRange = frameView.substr(endOffset);

                        int isRange = 1;
                        size_t lineCount = (paddedStart) ? IndividualReport::startLine - 1 : IndividualReport::startLine;
                        bool linePrefix = true;
                        for (const auto &text : {preRange, range, postRange}) {
                            size_t startByte = 0;
                            isRange = (isRange + 1) % 2;
                            while (startByte < text.size()) {
                                if (linePrefix) {
                                    std::string lineCountStr = std::to_string(lineCount + 1);
                                    padLeft(lineCountStr, 4);
                                    IndividualReport::messageStream << colorSrc("\n    ", Console::Internal::Color::gray)
                                        << colorSrc(lineCountStr, Console::Internal::Color::gray)
                                        << colorSrc(":", Console::Internal::Color::gray);
                                    linePrefix = false;
                                }

                                // Find the next newline
                                size_t endByte = text.find('\n', startByte);

                                // If no newline is found, clamp to the end of the string
                                if (endByte == std::string_view::npos) {
                                    endByte = text.size();
                                } else {
                                    lineCount++;
                                    linePrefix = true;
                                }

                                std::string_view line = text.substr(startByte, endByte - startByte);
                                if (isRange == 1 && shouldColor) {
                                    IndividualReport::messageStream << colorSrc((std::string) line, color);
                                } else {
                                    IndividualReport::messageStream << line;
                                }

                                // Move the start cursor past the newline character
                                startByte = endByte + 1;
                            }
                        }
                    }

                    // Print source
                    INTERNAL_C_OUT(IndividualReport::messageStream.str(), channel);
                }

                // Update print statistics
                if (isFirstPrint) {
                    isFirstPrint = false;
                }
            }
        }
    }
}
