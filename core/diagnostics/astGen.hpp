/// @file
/// @ingroup CoreDiagnostics
/// @brief Declares helpers that construct source-located #Diagnostics::Diagnostic instances from ANTLR4 parse-tree elements.
///
/// Used during AST generation to pre-populate a #Diagnostics::Diagnostic's #Diagnostics::Range
/// from an `antlr4::Token`, `antlr4::tree::TerminalNode`, or `antlr4::ParserRuleContext` before
/// applying #USE_CODE.
///
/// @see Diagnostics::Diagnostic
/// @see USE_CODE

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// ANTLR4 imports
#include "../parser/antlr4.hpp"

// Diagnostics
#include "Diagnostic.hpp"

namespace Diagnostics {

    /// @addtogroup CoreDiagnostics
    /// @{

    /// @brief Creates a #Diagnostic pre-populated with the 0-based source #Range of @p token.
    ///
    /// @param[in] token Pointer to the ANTLR4 token to extract line and column bounds from, or `nullptr`.
    /// @return A default-initialized #Diagnostic with `range` set to @p token's span (or `(0,0)-(0,0)` if @p token is `nullptr`).
    ///
    /// @see getGenNodeDiagnostic
    /// @see getGenRuleDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API Diagnostic getGenTokenDiagnostic(antlr4::Token *token) ;

    /// @brief Creates a #Diagnostic pre-populated with the 0-based source #Range of a parse-tree terminal @p node.
    ///
    /// @param[in] node Pointer to the ANTLR4 terminal node whose underlying symbol token provides the span, or `nullptr`.
    /// @return A #Diagnostic with `range` set to @p node's token span.
    ///
    /// @see getGenTokenDiagnostic
    /// @see getGenRuleDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API Diagnostic getGenNodeDiagnostic(antlr4::tree::TerminalNode *node) ;

    /// @brief Creates a #Diagnostic pre-populated with the 0-based source #Range spanning @p context's start and stop tokens.
    ///
    /// @param[in] context Pointer to the ANTLR4 parser rule context to extract start/stop token bounds from, or `nullptr`.
    /// @return A #Diagnostic with `range` spanning from `context->getStart()` to the end of `context->getStop()`.
    ///
    /// @see getGenTokenDiagnostic
    /// @see getGenNodeDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API Diagnostic getGenRuleDiagnostic(antlr4::ParserRuleContext *context) ;

    /// @}
}
