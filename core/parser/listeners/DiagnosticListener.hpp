/**
 * @brief
 * Lexer & parser error listeners
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"

// Diagnostics
#include "../../diagnostics/Diagnostic.hpp"

namespace Parser {
    namespace Listeners {
        // Track errors
        extern JUG_PARSER_API bool errorsDetected;

        // Listen for syntax-related errors
        class JUG_PARSER_API DiagnosticListener : public antlr4::BaseErrorListener {
            private:
                // ANTLR4 functions
                void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                    size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override ;
                void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
                    size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs)
                    override ;
                void reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
                    antlr4::atn::ATNConfigSet *configs) override ;
                void reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, size_t prediction,
                    antlr4::atn::ATNConfigSet *configs) override ;
            public:
                // Constructors
                DiagnosticListener() = default;
                DiagnosticListener(const DiagnosticListener&) = delete;
                DiagnosticListener& operator=(const DiagnosticListener&) = delete;

                virtual void onSyntaxError(Diagnostics::Diagnostic diag) = 0;
                virtual void onAmbiguity(Diagnostics::Diagnostic diag) = 0;
                virtual void onAttemptingFullContext(Diagnostics::Diagnostic diag) {};
                virtual void onContextSensitivity(Diagnostics::Diagnostic diag) {};
        };
    }
}
