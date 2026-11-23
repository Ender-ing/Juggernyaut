/**
 * @brief
 * Core diagnostic data structures
**/

#pragma once

#include "common/headers.hpp"
//#include "dynamic.hpp" // JUG_DATA_LIBRARY_EXPORTS

// Generated codes (.diag files)
#include "DiagCodes.hpp"

// Relative origin types
#include "../host/VFS.hpp"

#define SET_CODE(DIAG, SEVERITY, NUM, ...) \
    Data::Diagnostics::Diagnostic DIAG; \
    (DIAG).severity = Data::Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))

#define USE_CODE(DIAG, SEVERITY, NUM, ...) \
    (DIAG).severity = Data::Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))


namespace Data::Diagnostics {
    using File = Data::Host::File;
    using FileId = Data::Host::FileId;
    using GenericEvent = std::function<void()>;
    using FileEvent = std::function<void(const Data::Host::File*)>;

    struct Position {
        uint32_t line = 0;
        uint32_t character = 0;

        bool operator==(const Position&) const = default;
    };

    // The exact position within the final source
    struct Range {
        Position start;
        Position end;
    };

    enum Severity {
	    Error = 1,
    	Warning = 2,
	    Info = 3,
    	Hint = 4
    };

    // A call stack trace layer!
    /*struct Trace {
        std::string uri;
        Range position;
        std::string description;
    };*/
    // Extra information!
    /*struct RelatedInformation {
        std::string uri;
        Range position;
        Severity severity;
        std::string message;
    };*/

    struct Diagnostic {
        // Source info
        const File* file = nullptr; // Originating source #File, or `nullptr` for component-wide diagnostics.
        Range range;                // Source #Range within #file where the issue occurred.

        // Content
        std::string message = "";           // Human-readable diagnostic message text.
        int code = 1;                       // Numeric diagnostic code (see `DiagCodes.hpp`).
        Severity severity = Severity::Hint; // Impact level of this diagnostic.
        // std::vector<Trace> stack;
        // std::vector<RelatedInformation> related;

        // Flags
        bool isUnnecessary = false; // Used to report unnecessary/unused source ranges
    };

    using Event = std::function<void(const Diagnostic&)>;

    using ModifiableEvent = std::function<void(Diagnostic&)>; // Only for internal manipulation

    /*struct FileDiagnostics {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
    };*/
}
