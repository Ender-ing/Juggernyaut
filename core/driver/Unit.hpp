/**
 * @brief 
 * Front-End Driver Units (Sessions)
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DRIVER_API

// Sessions
#include "../session/Session.hpp"
namespace Driver {
    namespace JugDriver {
        using Unit = Session::Session;
        using UnitId = uint32_t;
        using UnitConfigs = Session::Configs;
        using UnitHooks = Session::Hooks;
    }
}
