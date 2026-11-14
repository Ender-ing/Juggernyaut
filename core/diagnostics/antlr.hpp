/// @file
/// @ingroup CoreDiagnostics
/// @brief Declares conversion helpers that translate ANTLR4 lexer/parser errors and ambiguities into #Diagnostics::Diagnostic instances.
///
/// @see Diagnostics::Diagnostic
/// @see astGen.hpp

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_DIAGNOSTICS_API

// ANTLR4 imports
#include "../parser/antlr4.hpp"

#include "Diagnostic.hpp"

namespace Diagnostics {

    /// @addtogroup CoreDiagnostics
    /// @{

    /// @brief Resolves a Juggernyaut numeric diagnostic code from an ANTLR4 exception or error message.
    ///
    /// Inspects @p e for known ANTLR4 exception types (`LexerNoViableAltException`,
    /// `InputMismatchException`, `NoViableAltException`, `FailedPredicateException`,
    /// `RecognitionException`), falling back to message-pattern or numeric-code parsing when @p e is null.
    ///
    /// @param[in]  msg      Raw error message string emitted by ANTLR4 (or a stringified numeric code).
    /// @param[in]  e        Exception pointer captured by the ANTLR4 recognizer, or `nullptr`.
    /// @param[out] isMsgNum Set to `true` if @p msg was a raw numeric code string whose message must be looked up from `DiagCodes.hpp`.
    ///
    /// @return The resolved Juggernyaut diagnostic code (`1xxxxx` for lexer, `2xxxxx` for parser).
    ///
    /// @see antlrSyntaxErrorToDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API int getAntlrErrorCode(const std::string &msg, std::exception_ptr e, bool &isMsgNum) ;

    /// @brief Converts an ANTLR4 syntax error callback payload into a populated #Diagnostic.
    ///
    /// Computes the multi-line or single-line #Range from @p offendingSymbol (converting ANTLR4's
    /// 1-based line numbers into 0-based #Position coordinates) and assigns a diagnostic code via #getAntlrErrorCode().
    ///
    /// @param[in] offendingSymbol    The ANTLR4 token where the syntax error occurred, or `nullptr` for lexer errors.
    /// @param[in] line               1-based line number reported by ANTLR4.
    /// @param[in] charPositionInLine 0-based column offset in @p line reported by ANTLR4.
    /// @param[in] msg                Error message or numeric diagnostic code string from ANTLR4.
    /// @param[in] e                  Captured ANTLR4 recognition exception pointer, or `nullptr`.
    ///
    /// @return A #Diagnostic with #Severity::Error and the computed source #Range.
    ///
    /// @see getAntlrErrorCode
    /// @see antlrAmbiguityToDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API Diagnostic antlrSyntaxErrorToDiagnostic(antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
        const std::string &msg, std::exception_ptr e) ;

    /// @brief Converts an ANTLR4 grammar ambiguity notification into a warning #Diagnostic.
    ///
    /// Extracts the start and stop tokens from @p recognizer's token stream over `[startIndex, stopIndex]`
    /// to build the spanning #Range and assigns diagnostic code `200002` (#Severity::Warning).
    ///
    /// @param[in] recognizer The active ANTLR4 parser instance.
    /// @param[in] dfa        The DFA state where the ambiguity was detected.
    /// @param[in] startIndex Token stream index of the first token in the ambiguous phrase.
    /// @param[in] stopIndex  Token stream index of the last token in the ambiguous phrase.
    /// @param[in] exact      `true` if the ambiguity is exact (full-context), `false` otherwise.
    /// @param[in] ambigAlts  BitSet of ambiguous alternative numbers.
    /// @param[in] configs    The ATN configuration set at the ambiguity point.
    ///
    /// @return A #Diagnostic with #Severity::Warning covering the ambiguous token interval.
    ///
    /// @see antlrSyntaxErrorToDiagnostic
    /// @see Diagnostic
    extern JUG_DIAGNOSTICS_API Diagnostic antlrAmbiguityToDiagnostic(antlr4::Parser *recognizer,
        const antlr4::dfa::DFA &dfa, size_t startIndex, size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts,
        antlr4::atn::ATNConfigSet *configs);

    /// @}
}
