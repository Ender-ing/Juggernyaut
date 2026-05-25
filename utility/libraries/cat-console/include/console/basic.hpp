/**
 * @brief
 * Include all used CLI headers
**/

#pragma once

#include "common/headers.hpp"
#include "../../dynamic.hpp" // JUG_CAT_CONSOLE_API

namespace Console {
    namespace Internal {
        namespace Color {
            extern JUG_CAT_CONSOLE_API const uint32_t blue_violet;
            extern JUG_CAT_CONSOLE_API const uint32_t sea_green;
            extern JUG_CAT_CONSOLE_API const uint32_t red;
            extern JUG_CAT_CONSOLE_API const uint32_t crimson;
            extern JUG_CAT_CONSOLE_API const uint32_t white;
            extern JUG_CAT_CONSOLE_API const uint32_t golden_rod;
            extern JUG_CAT_CONSOLE_API const uint32_t light_sea_green;
            extern JUG_CAT_CONSOLE_API const uint32_t gray;
        }
        extern JUG_CAT_CONSOLE_API std::string color(const std::string &text, const uint32_t color) ;
    }
}
