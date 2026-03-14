/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

namespace Parser {
    namespace Debug {
        // Check for syntax errors
        extern JUG_PARSER_API bool syntaxCheck (std::string file_contents) ;
    }
}
