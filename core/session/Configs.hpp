/**
 * @brief 
 * Front-End Session Configs
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_SESSION_API

// Host & Environment
#include "../data/host/VFS.hpp"
#include "../data/driver/Environment.hpp"

namespace Session {
    enum class Stage : uint8_t {
        Unprocessed = 0,
        Lexer       = 1,
        Parser      = 2,
        Transformer = 3,
        Semantic    = 4,
        IaRC        = 5
    };

    struct Configs {
        bool inheritEnvironmentBuildInputs = true;
        Stage terminateAfter = Stage::IaRC;
    };
}
