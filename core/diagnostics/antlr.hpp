/**
 * @brief
 * Handle antlr4 diagnostic conversion
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// ANTLR4 imports
#include "../parser/antlr4.hpp"

#include "Diagnostic.hpp"

namespace Diagnostics {
    extern JUG_DIAGNOSTICS_API int getAntlrErrorCode(const std::string &msg, std::exception_ptr e, bool &isMsgNum) ;
    extern JUG_DIAGNOSTICS_API Diagnostic antlrSyntaxErrorToDiagnostic(antlr4::Recognizer *recognizer,
        antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
        const std::string &msg, std::exception_ptr e) ;
    extern JUG_DIAGNOSTICS_API Diagnostic antlrAmbiguityToDiagnostic(antlr4::Parser *recognizer,
        const antlr4::dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts,
        antlr4::atn::ATNConfigSet *configs);
}
