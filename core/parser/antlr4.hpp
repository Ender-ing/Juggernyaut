/**
 * @brief
 * Fix ANTLR4 inclusion
**/

#pragma once

#ifdef _MSC_VER
    #pragma warning(push, 0) // Sets warning level to 0 (off) for everything below
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC system_header // Tells GCC/Clang to treat everything below as a system header
#endif

// Include the actual ANTLR runtime
#include "antlr4-runtime.h"

// Restore state
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
