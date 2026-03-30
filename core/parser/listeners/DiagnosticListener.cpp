/**
 * @brief
 * Lexer & parser error listeners
**/

#include "DiagnosticListener.hpp"

#include "../antlr4.hpp"

#include "../../common/utility.hpp"

// Diagnostics
#include "../../diagnostics/antlr.hpp"

namespace Parser {
    namespace Listeners {
        // Track errors
        bool errorsDetected = false;

        // Listen for syntax-related errors
        void DiagnosticListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
            size_t charPositionInLine, const std::string &msg, std::exception_ptr e) {

            errorsDetected = true;

            Diagnostics::Diagnostic diag = Diagnostics::antlrSyntaxErrorToDiagnostic(recognizer, offendingSymbol, line,
                charPositionInLine, msg, e);

            this->onSyntaxError(std::move(diag));
        }

        // TMP
        void DiagnosticListener::reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs) {

            Diagnostics::Diagnostic diag = Diagnostics::antlrAmbiguityToDiagnostic(recognizer, dfa, startIndex,
                stopIndex, exact, ambigAlts, configs);

            this->onAmbiguity(std::move(diag));
        }
        void DiagnosticListener::reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, const antlrcpp::BitSet &conflictingAlts, antlr4::atn::ATNConfigSet *configs) { }
        void DiagnosticListener::reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, size_t prediction, antlr4::atn::ATNConfigSet *configs) { }
    }
}
