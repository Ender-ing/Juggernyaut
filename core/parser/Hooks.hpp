/**
 * @brief 
 * Parser hooks
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Basic C++ headers
#include <functional>

//// Data
//#include "../data/store/Source.hpp"

#include "listeners/DiagnosticListener.hpp"

namespace Parser {
    typedef std::function<void(const std::string)> TokenReport;
    typedef std::function<void(const std::string)> TreeReport;

    // typedef std::function<Data::Store::Source(const std::string)> SourceRequest;

    struct Hooks {
        // [TokenReport]
        // Args: <token_text> (const std::string)
        TokenReport onANTLRTokenDetected = nullptr;
        // [TreeReport]
        // Args: <AST_text> (const std::string)
        TreeReport onANTLRTreeGenerated = nullptr;

        //// [SourceRequest]
        //// Args: <file_uri> (const std::string)
        //// Returns: <file_source_obj> (Data::Store::Source)
        // SourceRequest onSourceRequest = nullptr;
    };
}
