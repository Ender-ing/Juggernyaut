/**
 * @brief 
 * String-related functions
**/

#include "utility.hpp"

// Basic C++ headers
#include <algorithm>

namespace Common {
    namespace Utility {
        bool isNumber(const std::string &text) {
          return !text.empty() && std::all_of(text.begin(), text.end(), ::isdigit);
        }
    }
}
