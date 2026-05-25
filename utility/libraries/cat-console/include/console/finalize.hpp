/**
 * @brief
 * Handle CLI initialisation
**/

#pragma once

#include "common/headers.hpp"
#include "../../dynamic.hpp" // JUG_CAT_CONSOLE_API

namespace Console {
    namespace Internal {
        // Handle CLI initialisation
        extern JUG_CAT_CONSOLE_API void finalize(uint32_t activeSrcs) ;
    }
}
