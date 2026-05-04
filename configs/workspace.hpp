/**
 * @brief
 * Handle .toml workspace configs
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_CONFIGS_API

// Diagnostics
#include "../core/diagnostics/Diagnostic.hpp"

// Session
#include "../core/session/session.hpp"

// toml++
#include "tomlpp.hpp"

#include "BreakingChanges.hpp"

namespace Configs {
    namespace Workspace {
        BreakingChanges importDirs(Session::Session &session, std::vector<Diagnostics::Diagnostic> &diags, bool isStrict,
            toml::table &workspace, const std::string &rootUri) ;
    }
}
