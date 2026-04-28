/**
 * @brief
 * googletest
**/

#pragma once

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 6326) // Potential comparison of a constant with another constant
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
#endif

#include <gtest/gtest.h>

#if defined(_MSC_VER)
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
