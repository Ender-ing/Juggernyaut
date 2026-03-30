/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Basic C++ headers
#include <functional>

// Parser
#include "listeners/DiagnosticListener.hpp"

namespace Parser {
    namespace Debug {
        typedef std::function<void(const std::string)> TokenReport;
        typedef std::function<void(const std::string)> TreeReport;

        // Check for syntax errors
        extern JUG_PARSER_API bool syntaxCheck(const std::string &file_contents, const TokenReport &onTokenCall,
            const TreeReport &onTreeCall,
            Listeners::DiagnosticListener *lexerErrorListener, Listeners::DiagnosticListener *parserErrorListener) ;
    }
}
