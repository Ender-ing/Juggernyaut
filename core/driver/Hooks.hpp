/**
 * @brief 
 * Front-End Driver Hooks
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DRIVER_API

#include "../diagnostics/Diagnostic.hpp"

namespace Driver {
    using Diagnostic = Diagnostics::Diagnostic;
    using UnitSourceId = Data::Host::FileId;

    using GenericEvent = Diagnostics::GenericEvent;
    using DriverEvent = Diagnostics::Event;
    using FileEvent = Diagnostics::FileEvent;
    // using UnitSourceStringEvent = std::function<void(const Unit&, const std::string&)>;
    // using UnitSourceU8tStringEvent = std::function<void(const Unit&, const uint8_t&, const std::string&)>;

    // Unit's source-level hooks
    struct UnitSourceHooks {
        // Source stages
        // UnitSourceEvent onContextSkip = nullptr;
        FileEvent onContextStart = nullptr;
        FileEvent onLexerContextEnd = nullptr;
        FileEvent onParserContextEnd = nullptr;
        FileEvent onContextEnd = nullptr;
    };

    // Unit-level hooks
    struct UnitHooks {
        // ...
    };

    // Driver-level hooks
    struct DriverHooks {
        Diagnostics::Event onDiagnostic = nullptr; // ALL Diagnostics go through this function!
        GenericEvent onRunFinished = nullptr;
        FileEvent onEnvironmentLoad = nullptr;
        FileEvent onEnvironmentChange = nullptr;
    };

    struct Hooks {
        DriverHooks driver;
        UnitHooks units;
        UnitSourceHooks unitSources;
    };
}
