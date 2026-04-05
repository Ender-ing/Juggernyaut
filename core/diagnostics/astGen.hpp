/**
 * @brief
 * Lexer & parser error listeners
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// ANTLR4 imports
#include "../parser/antlr4.hpp"

// Diagnostics
#include "Diagnostic.hpp"

namespace Diagnostics {
    extern JUG_DIAGNOSTICS_API Diagnostic getGenTokenDiagnostic(antlr4::Token *token) ;
    extern JUG_DIAGNOSTICS_API Diagnostic getGenNodeDiagnostic(antlr4::tree::TerminalNode *node) ;
    extern JUG_DIAGNOSTICS_API Diagnostic getGenRuleDiagnostic(antlr4::ParserRuleContext *context) ;
}
