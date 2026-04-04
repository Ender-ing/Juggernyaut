/**
 * @brief
 * Handle CLI report displays
**/

#include "report.hpp"

// Comms headers
#include "console.hpp"
#include "basic.hpp"

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
                        prompt << " (" << IndividualReport::stage << ") ";
                    } else {
                        prompt << " ";
                    }
                    out(prompt.str());
                }

                // TMP
                out(IndividualReport::messageStream.str());

                // Update print statistics
                if (isFirstPrint) {
                    isFirstPrint = false;
                }
            }
        }
    }
}
