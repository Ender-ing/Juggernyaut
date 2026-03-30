/**
 * @brief 
 * String-related functions
**/

#pragma once

#include "headers.hpp"
#include "dynamic.hpp" // JUG_COMMON_API

namespace Common {
    namespace Utility {
        template <typename Target, typename Source>
        inline bool isType(Source* obj) {
            // We add const to the target so it works with const Source pointers
            return dynamic_cast<const Target*>(obj) != nullptr;
        }

        extern JUG_COMMON_API bool isNumber(const std::string &text) ;
    }
}
