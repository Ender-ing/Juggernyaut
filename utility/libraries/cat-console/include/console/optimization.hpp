/**
 * @brief
 * Include all used report headers
**/

#pragma once

#include "common/headers.hpp"
#include "../../dynamic.hpp" // JUG_CAT_CONSOLE_API

namespace Console {
    namespace Optimization {
        extern JUG_CAT_CONSOLE_API void optimize() ;
        extern JUG_CAT_CONSOLE_API void revert() ;
        extern JUG_CAT_CONSOLE_API void safetyCheck() ;
    }
}
