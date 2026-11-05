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
    using FileEvent = Diagnostics::FileEvent;
    class JUG_SESSION_API Session;
    using StageCall = std::function<void(const Session*)>;

    struct Hooks {
        // [Session Lifecycle]
        StageCall onRunFinished = nullptr; // The Session is done running!

        // [Source Stage Lifecycle]
        FileEvent onContextSkip = nullptr;
        FileEvent onContextStart = nullptr;
        FileEvent onLexerContextEnd = nullptr;
        FileEvent onParserContextEnd = nullptr;
        FileEvent onContextEnd = nullptr;

        // [Diagnostics]
        Diagnostics::Event onDiagnostic = nullptr; // ALL session diagnostics go through here!
    };
}
