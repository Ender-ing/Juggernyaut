/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Parser
#include "Hooks.hpp"
#include "Configs.hpp"

namespace Parser {
    namespace Debug {
        // Check for syntax errors
        extern JUG_PARSER_API bool syntaxCheck(const std::string &file_contents, const TokenReport &onTokenCall,
            const TreeReport &onTreeCall,
            Listeners::DiagnosticListener *lexerErrorListener, Listeners::DiagnosticListener *parserErrorListener) ;
    }
    extern JUG_PARSER_API void sessionWorkflow(const Configs &configs, const Hooks &hooks) ;
}
