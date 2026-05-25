/**
 * @brief
 * Include all used report headers
**/

#pragma once

#include "common/headers.hpp"
#include "../../dynamic.hpp" // JUG_CAT_CONSOLE_API

namespace Console {
    namespace Internal {
        // Handle reports CLI outputs
        namespace Reports {
            // Print report details
            extern JUG_CAT_CONSOLE_API void print() ;
        }
    }
}
