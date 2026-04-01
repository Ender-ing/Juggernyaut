/**
 * @brief 
 * Parser configurations
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

namespace Parser {
    struct Configs {
        bool terminateAfterLexer = false;
    };
}
