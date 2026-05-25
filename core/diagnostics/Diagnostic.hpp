/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

#include "../data/store/types.hpp"

// Generated codes (.diag files)
#include "DiagCodes.hpp"

namespace Diagnostics {
    // Line/column data
    struct Position {
        uint32_t line;
        uint32_t character;

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
        Data::Store::SourceId sourceId = 0; // Keep as '0' for non-jug files
        Range range;
        std::string message;
        int code = 1;
        Severity severity;
        // std::vector<Trace> stack;
        // std::vector<RelatedInformation> related;
    };

    /*struct FileDiagnostics {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
    };*/
}
