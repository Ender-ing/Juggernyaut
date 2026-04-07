/**
 * @brief
 * ANTLR4 cleanup
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

namespace Parser {
    namespace Internal {
        extern JUG_PARSER_API void flushAntlrCache() ;
    }
}
