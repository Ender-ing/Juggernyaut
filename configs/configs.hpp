/**
 * @brief
 * Handle .toml configs
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_CONFIGS_API

// Session
#include "../core/session/session.hpp"

namespace Configs {
    extern JUG_CONFIGS_API bool modifySession(Session::Session &session, const std::string &configUri,
        std::string &errorLog) ;
}
