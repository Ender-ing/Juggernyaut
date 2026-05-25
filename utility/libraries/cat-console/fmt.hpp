/**
 * @brief
 * {fmt} wrapper
**/

#pragma once

// Disable warnings
#ifdef _MSC_VER
    #pragma warning(push, 0) // Sets warning level to 0 (off) for everything below
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC system_header // Tells GCC/Clang to treat everything below as a system header
#endif

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/std.h>

// Restore state
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
