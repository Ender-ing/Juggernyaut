/**
 * @brief
 * Lexer & parser error listeners
**/

#pragma once

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"
#include "JuggernyautParser.h"

// Diagnostics
#include "../../diagnostics/Diagnostic.hpp"

// Store
#include "../../data/store/SourceStore.hpp"

namespace Parser {
    namespace Listeners {
        class JUG_PARSER_API ASTGenListener {
            private:
                std::unique_ptr<Data::Store::Source> &src;
            public:
                ASTGenListener(std::unique_ptr<Data::Store::Source> &source)
                    : src(source) { }

                // (will attach a <SourceId>)
                void pushDiagnostic(Diagnostics::Diagnostic &diag) ;
        };
    }
}
