/**
 * @brief 
 * String-related functions
**/

#pragma once

#include "headers.hpp"
#include "../../dynamic.hpp" // JUG_COMMON_API

// Basic C++ libraries
#include <algorithm>

namespace Common {
    namespace Utility {
        template <typename Target, typename Source>
        inline bool isType(Source* obj) {
            // We add const to the target so it works with const Source pointers
            return dynamic_cast<const Target*>(obj) != nullptr;
        }

        template <typename Type>
        void fastVectorRemove(std::vector<Type> &vec, const Type &target) {
            auto it = std::find(vec.begin(), vec.end(), target);
            if (it != vec.end()) {
                // Swap the found element with the last element
                *it = vec.back(); 
                // Pop the last element off the end
                vec.pop_back();   
            }
        }

        template <typename Type>
        constexpr bool isInVector(const std::vector<Type> &vec, const Type &target) {
            return std::find(vec.begin(), vec.end(), target) != vec.end();
        }

        extern JUG_COMMON_API bool isNumber(const std::string &text) ;
    }
}
