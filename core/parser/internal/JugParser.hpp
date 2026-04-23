/**
 * @brief
 * Parser class
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"
#include "JuggernyautParser.h"

namespace Parser {
    namespace Internal {
        class JUG_PARSER_API JugParser : public GeneratedParser::JuggernyautParser {
            public:
                using GeneratedParser::JuggernyautParser::JuggernyautParser; // Inherit constructors
                JugParser(const JugParser&) = delete;
                JugParser& operator=(const JugParser&) = delete;
        };
    }
}
