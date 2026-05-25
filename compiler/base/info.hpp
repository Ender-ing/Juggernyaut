/**
 * @brief
 * Manage the compiler's basic info
**/

#pragma once

#include "common/headers.hpp"

namespace Base {
    namespace Info {
        // Version info
        extern const std::string version;

        // installation info
        extern const std::string arch;
        extern const std::string build;

        // OS info
        extern const std::string osName;
        extern const std::string osVersion;
    }
}
