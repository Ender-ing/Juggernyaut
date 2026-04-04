/**
 * @brief 
 * Manage compiler arguments and starting state
**/

#pragma once

#include "../../core/common/headers.hpp"

#include "actions.hpp"

// Basic C++ headers
//#include <string>

// WORK IN PROGRESS

namespace Base {
    // All state-related members should be contained under one namepsace
    namespace InitialConfigs {
        // Starting Path
        extern std::string compilerBinPath;

        // Main source file
        extern std::string mainPath;
        extern std::vector<std::string> entryPaths;

        // Debug-related
        namespace Debug {
            // --debug-antlr-syntax
            namespace Parser {
                extern bool activateAntlrSyntaxTest;
            }
        }

        // Technical values
        namespace Technical {
            // For actions that require termination after the arguments are fully processed!
            extern bool terminateAfterArgs;
            // For actions that require termination after actions!
            extern bool terminateAfterActions;
            // Fail process when unknown flags are detected!
            extern bool strictFlagDetection;

            // Look for flags that require the default initialisation to stop!
            // [true - skip, false - don't skip]
            extern bool shouldSkipDefaultInitialization(int argc, const char *argv[]) ;
        }

        // Process and update values through program arguments!
        // [true - sucess, false - failure]
        extern bool updateUsingArgs(int argc, const char *argv[]) ;
    }
}
