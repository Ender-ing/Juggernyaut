/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// Generated codes (.diag files)
#include "DiagCodes.hpp"

// Relative origin types
#include "../data/host/VFS.hpp"

// Code shorthand
#define SET_CODE(DIAG, SEVERITY, NUM, ...) \
    Diagnostics::Diagnostic DIAG; \
    (DIAG).severity = Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))

#define USE_CODE(DIAG, SEVERITY, NUM, ...) \
    (DIAG).severity = Diagnostics::Severity::SEVERITY; \
    (DIAG).code = (NUM); \
    (DIAG).message = CODE_##NUM __VA_OPT__((__VA_ARGS__))


namespace Diagnostics {
    // Basic data types
    using File = Data::Host::File;
    using FileId = Data::Host::FileId;

    // Event types
    using GenericEvent = std::function<void()>;
    using FileEvent = std::function<void(const Data::Host::File*)>;

    // Line/column data
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
    // Report severity
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
    // A report
    struct Diagnostic {
        // Source info
        const File* file = nullptr;
        Range range;

        // Content
        std::string message = "";
        int code = 1;
        Severity severity = Severity::Hint;
        // std::vector<Trace> stack;
        // std::vector<RelatedInformation> related;
    };
    using Event = std::function<void(const Diagnostic&)>;
    using ModifiableEvent = std::function<void(Diagnostic&)>; // Only for internal manipulation

    /*struct FileDiagnostics {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
    };*/
}
