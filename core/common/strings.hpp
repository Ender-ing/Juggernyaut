/**
 * @brief 
 * String-related functions
**/

#pragma once

#include "headers.hpp"
#include "dynamic.hpp" // JUG_COMMON_API

namespace Common {
    namespace Strings {
        // Convert a std::string into lowercase format
        extern JUG_COMMON_API void toLowerCase(std::string &str) ;

        // Make a lowercase copy of a std::string
        extern JUG_COMMON_API std::string copyToLowerCase(const std::string &str) ;
    }
}
