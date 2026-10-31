/**
 * @brief 
 * Session-specific hooks
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_SESSION_API

// Diagnostics
#include "../diagnostics/Diagnostic.hpp"

namespace Session {
    class JUG_SESSION_API Session;
    using StageCall = std::function<void(const Session*)>;

    struct Hooks {
        // [StageCall]
        StageCall onRunFinished = nullptr; // The Session is done running!

        Diagnostics::Event onDiagnostic = nullptr; // ALL session diagnostics go through here!
    };
}
