/**
 * @brief
 * Handle CLI initialisation
**/

#pragma once

#include "common/headers.hpp"
#include "../../dynamic.hpp" // JUG_CAT_CONSOLE_API

// Basic C++ headers
#include <chrono>

namespace Console {
    namespace Internal {
        extern JUG_CAT_CONSOLE_API bool isTrackingTime;
        extern JUG_CAT_CONSOLE_API std::chrono::high_resolution_clock::time_point start;

        extern JUG_CAT_CONSOLE_API void startClock() ;

        struct InitializationData {
            // General info
            const std::string name;
            const std::string version;
            const std::string build;
            const std::string arch;

            // Platform info
            const std::string osName;
            const std::string osVersion;
        };
        // Handle CLI initialisation
        extern JUG_CAT_CONSOLE_API void initialize(const InitializationData &data, const std::string &extra = "(C) 2026 Ender-ing GitHub Organisation") ;
    }
}
