/**
 * @brief
 * Manage file printing
**/

#pragma once

#include "common/headers.hpp"

namespace Console {
    namespace Source {
        struct Offsets {
            size_t start = 0;
            size_t end = 0;
        };
        extern std::string extractRange(const std::string& path, size_t startLine, size_t endLine, bool &paddedStart) ;
    }
}