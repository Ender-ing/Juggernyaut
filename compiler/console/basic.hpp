/**
 * @brief
 * Include all used CLI headers
**/

#pragma once

#include "common/headers.hpp"

namespace Console {
    namespace Internal {
        namespace Color {
            extern const uint32_t blue_violet;
            extern const uint32_t sea_green;
            extern const uint32_t red;
            extern const uint32_t crimson;
            extern const uint32_t white;
            extern const uint32_t golden_rod;
            extern const uint32_t light_sea_green;
        }
        extern std::string color(const std::string &text, const uint32_t color) ;
    }
}
