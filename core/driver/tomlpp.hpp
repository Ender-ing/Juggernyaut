/**
 * @brief
 * Handle toml++ imports
**/

#pragma once

// Disable warnings
#ifdef _MSC_VER
    #pragma warning(push, 0) // Sets warning level to 0 (off) for everything below
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC system_header // Tells GCC/Clang to treat everything below as a system header
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
