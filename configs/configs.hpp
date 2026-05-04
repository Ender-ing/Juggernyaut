/**
 * @brief
 * Handle .toml configs
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_CONFIGS_API

// Diagnostics
#include "../core/diagnostics/Diagnostic.hpp"

// Session
#include "../core/session/session.hpp"

#include "BreakingChanges.hpp"

namespace Configs {
    // [modifySession]
    // Args: <session> (Session::Session - modified)
    //       <configUri> (const std::string&)
    //       <diags> (std::vector<Diagnostics::Diagnostic> - modified)
    //       <isStrict> (bool - triggers 'strict session modifications')
    //       <breaking> (BreakingChanges* - modified - signals breaking changes within the session)
    // Returns: <is_success> (bool)
    extern JUG_CONFIGS_API bool modifySession(Session::Session &session, const std::string &configUri,
        std::vector<Diagnostics::Diagnostic> &diags, bool isStrict, BreakingChanges *breaking = nullptr) ;
    // [resetSessionConfigs]
    // Args: <session> (Session::Session - modified)
    // Meant to reset configs changed by 'modifySession'!
    // Returns: <breaking_changes> (BreakingChanges)
    extern JUG_CONFIGS_API BreakingChanges resetSessionConfigs(Session::Session &session) ;
    // [refreshSessionState]
    // Args: <session> (Session::Session - modified)
    //       <changes> (BreakingChanges - changes you want to base this refresh on)
    extern JUG_CONFIGS_API void refreshSessionState(Session::Session &session, BreakingChanges changes) ;
}
