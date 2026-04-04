/**
 * @brief 
 * Parser hooks
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Diagnostics
#include "../diagnostics/Diagnostic.hpp"

namespace Parser {
    typedef std::function<void(const std::string&)> TokenReport;
    typedef std::function<void(const std::string&)> TreeReport;
    typedef std::function<void(const Data::Store::SourceId)> StageCall;

    typedef std::function<void(const Diagnostics::Diagnostic&)> ParserDiagEvent;

    // typedef std::function<Data::Store::Source(const std::string)> SourceRequest;

    struct Hooks {
        // [StageCall]
        StageCall onContextStart = nullptr;
        StageCall onLexerContextEnd = nullptr;
        StageCall onContextEnd = nullptr;
        // [TokenReport]
        // Args: <token_text> (const std::string&)
        TokenReport onANTLRTokenDetected = nullptr;
        // [TreeReport]
        // Args: <AST_text> (const std::string&)
        TreeReport onANTLRTreeGenerated = nullptr;

        // [ParserDiagEvent]
        // Args: <diagnostics_obj> (Diagnostics::Diagnostic)
        ParserDiagEvent onSyntaxError = nullptr;
        ParserDiagEvent onAmbiguity = nullptr;
        ParserDiagEvent onAttemptingFullContext = nullptr;
        ParserDiagEvent onContextSensitivity = nullptr;

        //// [SourceRequest]
        //// Args: <file_uri> (const std::string)
        //// Returns: <file_source_obj> (Data::Store::Source)
        // SourceRequest onSourceRequest = nullptr;
    };
}
