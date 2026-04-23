/**
 * @brief 
 * Parser configurations
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

#include "listeners/DiagnosticListener.hpp"

namespace Parser {
    struct Configs {
        bool terminateAfterLexer = false;
    };
    struct SharedConfigs {
        // ...
    };
}
