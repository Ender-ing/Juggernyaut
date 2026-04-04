/**
 * @brief
 * Manage compiler arguments and starting state
**/

#include "config.hpp"
#include "../../core/common/strings.hpp"

#include "../console/console.hpp"

#include "../store/files.hpp"

namespace Base {
    // All state-related members should be contained under one namepsace
    namespace InitialConfigs {
        // Starting Path
        std::string compilerBinPath = "";

        // Main source file
        std::string mainPath = "";
        std::vector<std::string> entryPaths;

        // Debug-related
        namespace Debug {
            // --debug-parser-antlr-syntax-test <path>
            namespace Parser {
                bool activateAntlrSyntaxTest = false;
            }
        }

        // Technical values
        namespace Technical {
            // For actions that require termination after the arguments are fully processed!
            bool terminateAfterArgs = false;
            // For actions that require termination after actions!
            bool terminateAfterActions = false;
            // Fail process when unknown flags are detected!
            bool strictFlagDetection = false;

            // Look for flags that require the default initialisation to stop!
            // [true - skip, false - don't skip]
            bool shouldSkipDefaultInitialization(int argc, const char *argv[]) {
                // List of flags
                std::vector<std::string> flagsList{"-v", "--version"};
                // Loop through all arguments (skipping the first one)
                for (int i = 1; i < argc; i++) {
                    // Get the current argument
                    std::string arg (argv[i]);
                    // Convert the flag into lowercase format
                    Common::Strings::toLowerCase(arg);

                    // Check if the flag is in the list
                    if (std::find(std::begin(flagsList), std::end(flagsList), arg) != std::end(flagsList)) {
                        return true;
                    }

                }

                // Don't skip!
                return false;
            }
        }

        // Process and update values through program arguments!
        // [true - sucess, false - failure]
        bool updateUsingArgs(int argc, const char *argv[]) {
            // Get the compiler's path
            if (!Store::getExecutableDir(compilerBinPath)) { // Same as /path/to/bin
                REPORT(Console::START_REPORT, Console::FATAL_REPORT,
                    "Couldn't get the compiler's path!",
                    BAD_CODE_OR_MEMORY_LEAKS,
                    Console::END_REPORT);
            }
            compilerBinPath = Store::getParentPath(compilerBinPath);// Same as  /path/to/bin/..

            // Loop through all arguments (skipping the first one)
            for (int i = 1; i < argc; i++) {
                // Get the current argument
                std::string arg (argv[i]);
                // Convert the flag into lowercase format
                Common::Strings::toLowerCase(arg);

                // Use this function to get the next argument
                // [true - success, false = failure]
                const Actions::ActionNextFunction getNextArg = [&i, &argc, &argv](std::string *store, bool skip) {
                    // Check for the next argument
                    if (i + 1 < argc) {
                        if (store != nullptr) {
                            // Get the next argument (and skip it when necessary!)
                            *store = std::string(argv[i + 1]);
                        }
                        if (skip) {
                            i++;
                        }
                        return true;
                    } else {
                        return false;
                    }
                };

                // Check current flag
                Actions::ActionFunction action = nullptr;
                if (arg[0] != '-') {
                    // Unexpected input!
                    REPORT(Console::START_REPORT,
                        (InitialConfigs::Technical::strictFlagDetection) ? Console::FATAL_REPORT : Console::WARNING_REPORT,
                        "Unexpected command line input: '", arg, "'",
                        Console::END_REPORT);
                    if (InitialConfigs::Technical::strictFlagDetection) {
                        return false;
                    }
                } else if (Actions::getActionFunctionByFlag(arg, action)) {
                    // Execute action, and check for failure
                    if (action != nullptr && !action(getNextArg)) {
                        // Action-related fatal error!
                        // Error message is handled by the action!
                        return false;
                    }
                } else {
                    // Unknown argument!
                    REPORT(Console::START_REPORT,
                        (InitialConfigs::Technical::strictFlagDetection) ? Console::FATAL_REPORT : Console::WARNING_REPORT,
                        "Unknown command line flag: ", arg,
                        Console::END_REPORT);
                    if (InitialConfigs::Technical::strictFlagDetection) {
                        return false;
                    }
                }
            }

            // Success!
            return true;
        }
    }
}
