/**
 * @brief
 * Lexer class
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"
#include "JuggernyautLexer.h"

namespace Parser {
    namespace Internal {
        class JUG_PARSER_API JugLexer : public GeneratedLexer::JuggernyautLexer {
            public:
                using GeneratedLexer::JuggernyautLexer::JuggernyautLexer; // Inherit constructors

                void emit(std::unique_ptr<antlr4::Token> token) override ;
        };
    }
}
