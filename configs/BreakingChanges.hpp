/**
 * @brief
 * Handle .toml configs
**/

#pragma once

// #include "common/headers.hpp"
// #include "dynamic.hpp" // JUG_CONFIGS_API

namespace Configs {
    enum BreakingChanges {
        INVALID = 0,
        Parser = 1,
        None = 10 // Must be the highest!
    };
}
