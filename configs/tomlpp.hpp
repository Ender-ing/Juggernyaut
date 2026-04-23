/**
 * @brief
 * Handle toml++ imports
**/

#pragma once

// Disable known warnings
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 5311)  // Deprecated literal operator
    #pragma warning(disable : 6011)  // Dereferencing NULL pointer
    #pragma warning(disable : 28199) // Possibly uninitialized memory 'digits'
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #pragma GCC diagnostic ignored "-Wnull-dereference"
    #ifndef __apple_build_version__
        #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
    #endif
#endif

// Imports
#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

// Restore state
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
