/**
 * @brief 
 * Parser hooks
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Diagnostics
#include "../diagnostics/Diagnostic.hpp"

namespace Parser {
    using FileEvent = Diagnostics::FileEvent;

    // typedef std::function<Data::Store::Source(const std::string)> SourceRequest;

    struct Hooks {
        // [Source Stage Lifecycle]
        FileEvent onContextSkip = nullptr; // Context is already up-to-date
        FileEvent onContextStart = nullptr; // Fresh lex/parse started for file
        FileEvent onLexerContextEnd = nullptr; // Lexer stage finished for file
        FileEvent onParserContextEnd = nullptr; // Parser stage finished for file
        FileEvent onContextEnd = nullptr; // Full parser workflow finished for file

        // [Diagnostic Events]
        // Args: <diagnostics_obj> (Diagnostics::Diagnostic)
        Diagnostics::ModifiableEvent onDiagnostic = nullptr;
    };
}
