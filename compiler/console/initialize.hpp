/**
 * @brief
 * Handle CLI initialisation
**/

#pragma once

#include "common/headers.hpp"

// Basic C++ headers
#include <chrono>

namespace Console {
    namespace Internal {
        extern bool isTrackingTime;
        extern std::chrono::high_resolution_clock::time_point start;

        extern void startClock() ;
        // Handle CLI initialisation
        extern void initialize() ;
    }
}
