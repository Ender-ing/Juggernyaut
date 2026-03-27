/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// ANTLR4 imports
#include "../parser/antlr4.hpp"

namespace Diagnostics {
    // Line/column data
    struct Position {
        uint32_t line;
        uint32_t character;
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
        Range range;
        std::string message;
        int code;
        Severity severity;
        // std::vector<Trace> stack;
        // std::vector<RelatedInformation> related;
    };

    struct FileDiagnostics {
        std::string uri;
        std::vector<Diagnostic> diagnostics;
    };

    extern JUG_DIAGNOSTICS_API int getAntlrErrorCode(const std::string &msg, std::exception_ptr e) ;
    extern JUG_DIAGNOSTICS_API Diagnostic antlrToDiagnostic(antlr4::Recognizer *recognizer,
        antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
        const std::string &msg, std::exception_ptr e) ;
}
