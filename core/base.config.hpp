/**
 * @brief 
 * Manage compiler arguments and starting state
**/

#pragma once

#include "common/headers.hpp"
#include "base.dynamic.hpp" // JUG_BASE_API

#include "base.actions.hpp"

// Basic C++ headers
//#include <string>

// WORK IN PROGRESS

namespace Base {
    // Version info
    extern JUG_BASE_API const std::string version;

    // All state-related members should be contained under one namepsace
    namespace InitialConfigs {
        // Starting Path
        extern JUG_BASE_API std::string compilerBinPath;

        // Main source file
        extern JUG_BASE_API std::string mainPath;

        // Debug-related
        namespace Debug {
            // --debug-parser-antlr-syntax-test <path>
            namespace Parser {
                extern JUG_BASE_API bool activateAntlrSyntaxTest;
            }
        }

        // Technical values
        namespace Technical {
            // For actions that require termination after the arguments are fully processed!
            extern JUG_BASE_API bool terminateAfterArgs;
            // For actions that require termination after actions!
            extern JUG_BASE_API bool terminateAfterActions;
            // Fail process when unknown flags are detected!
            extern JUG_BASE_API bool strictFlagDetection;

            // Look for flags that require the default initialisation to stop!
            // [true - skip, false - don't skip]
            extern JUG_BASE_API bool shouldSkipDefaultInitialization(int argc, const char *argv[]) ;
        }

        // Process and update values through program arguments!
        // [true - sucess, false - failure]
        extern JUG_BASE_API bool updateUsingArgs(int argc, const char *argv[]) ;
    }
}
