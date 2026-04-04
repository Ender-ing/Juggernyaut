/**
 * @brief
 * Manage compiler actions
 * (The actual actions should be implemented somewhere else if possible!)
**/

#include "actions.hpp"

#include "config.hpp"
#include "info.hpp"

#include "../console/console.hpp"

#include "../store/files.hpp"

// Shorten the syntax for defining an action
#define DEFINE_ACTION(FLAG1, FLAG2, DESCRIPTION, ARGS, FUNCTION){   \
    {                                                               \
        "-" FLAG1,                                                  \
        "--" FLAG2,                                                 \
        DESCRIPTION                                                 \
    },                                                              \
    ARGS,                                                           \
    [](const ActionNextFunction getNextArg) FUNCTION                \
}                                                                   \

// Hide the return keyword to avoid confusion
#define ACTION_FATAL_FAILURE                return false
#define ACTION_PROGRESS                     return true

namespace Base {
    namespace Actions {
        // Actions list
        const ActionsList actions = {
            DEFINE_ACTION(
                "sf", "strict-flags",
                "Terminate process when unknown flags are detected.",
                {},
                {
                    REPORT(Console::START_REPORT, Console::ACTION_REPORT, "Enabled 'strict flags' mode!", Console::END_REPORT);

                    // Change to strict flags mode
                    InitialConfigs::Technical::strictFlagDetection = true;

                    ACTION_PROGRESS;
                }
            ),
            DEFINE_ACTION(
                "v", "version",
                "Get the plain version string. (No extra console output will be made as long as no errors occur)",
                {},
                {
                    REPORT(Console::START_REPORT, Console::NORMAL_REPORT, Info::version, Console::END_REPORT);

                    // Prevent other outputs
                    InitialConfigs::Technical::terminateAfterArgs = true;
                    Console::minimalProtocolFinalization = true;

                    ACTION_PROGRESS;
                }
            ),
            DEFINE_ACTION(
                "i", "input",
                "Set a path for the main user input file.",
                { "<path>" },
                {
                    // Store
                    std::string filePath;

                    // Get the next argument and save it!
                    bool success = getNextArg(&filePath, false);

                    // Check if the action was successful!
                    if (!success || filePath.at(0) == '-') {
                        // Missing input argument!
                        REPORT(Console::START_REPORT, Console::FATAL_REPORT,
                            "Missing the <path> input argument! (-i, --input)", Console::END_REPORT);
                        ACTION_FATAL_FAILURE;
                    }

                    do {
                        // Keep the value!
                        getNextArg(nullptr, true); // Skip the item
                        Base::InitialConfigs::entryPaths.push_back(filePath);
                    } while(getNextArg(&filePath, false) && filePath.at(0) != '-');

                    ACTION_PROGRESS;
                }
            ),
            DEFINE_ACTION(
                "l", "license",
                "Get license text.",
                {},
                {
                    // Report action
                    REPORT(Console::START_REPORT, Console::ACTION_REPORT, "Printing license text!",
                        Console::END_REPORT);

                    // Get the license text
                    std::string content;
                    std::string licensePath = InitialConfigs::compilerBinPath + "/LICENSE";
                    if (!Store::getFileContent(licensePath, content)) {
                        // File isn't accessible!
                        REPORT(Console::START_REPORT, Console::FATAL_REPORT,
                            "Couldn't access the LICENSE file: ",
                            licensePath,
                            BAD_CODE_OR_MEMORY_LEAKS,
                            Console::END_REPORT);

                        ACTION_FATAL_FAILURE;
                    }

                    // Print the license text
                    REPORT(Console::START_REPORT, Console::NORMAL_REPORT, content, Console::END_REPORT);

                    ACTION_PROGRESS;
                }
            ),
            DEFINE_ACTION(
                "dbg-antlr", "debug-antlr-syntax",
                "Print the parser's ANTLR4 tokens list and initial ANTLR4 tree.",
                {},
                {
                    // Enable the test
                    InitialConfigs::Debug::Parser::activateAntlrSyntaxTest = true;

                    // Report status
                    REPORT(Console::START_REPORT, Console::ACTION_REPORT, "Enabled the ANTLR4 parser print syntax test!",
                        Console::END_REPORT);

                    ACTION_PROGRESS;
                }
            ),
        };

        // The help menu!
        void printHelpMenu() {
            REPORT(Console::START_REPORT, Console::NORMAL_REPORT, "Options:\n");
            for (size_t i = 0; i < actions.size(); i++) {
                REPORT("    ", actions[i].info[0], "/", actions[i].info[1], " \t ", actions[i].info[2]);
                for (size_t j = 0; j < actions[i].input.size(); j++) {
                    REPORT("\n    \t", actions[i].input[j]);
                }
                REPORT("\n");
            }
            REPORT(Console::END_REPORT);
        }

        // Get an action function using one flag
        // [true - success, false - failure]
        bool getActionFunctionByFlag(const std::string& flag, ActionFunction &store) {
            if (flag == "-h" || flag == "--help") {
                printHelpMenu();
                return true;
            } else {
                for (size_t i = 0; i < actions.size(); i++) {
                    if (actions[i].info[0] == flag || actions[i].info[1] == flag) {
                        store = actions[i].func;
                        return true;
                    }
                }
            }
            return false;
        }
    }
}
