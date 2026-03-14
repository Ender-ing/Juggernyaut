/**
 * @brief
 * Include all used CLI headers
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_COMMS_API

// Basic C++ headers
#include <cstdint>

namespace Comms {
    namespace CLI {
        namespace Color {
            extern JUG_COMMS_API const uint32_t blue_violet;
            extern JUG_COMMS_API const uint32_t sea_green;
            extern JUG_COMMS_API const uint32_t red;
            extern JUG_COMMS_API const uint32_t crimson;
            extern JUG_COMMS_API const uint32_t white;
            extern JUG_COMMS_API const uint32_t golden_rod;
            extern JUG_COMMS_API const uint32_t light_sea_green;
        }
        extern JUG_COMMS_API std::string color(const std::string &text, const uint32_t color) ;
    }
}
