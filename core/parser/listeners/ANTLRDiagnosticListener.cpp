/**
 * @brief
 * ANTLR4 Lexer & Parser diagnostic listener
**/

#include "ANTLRDiagnosticListener.hpp"

// Diagnostics conversion helpers
#include "../../diagnostics/antlr.hpp"

namespace Parser::Listeners {
    void ANTLRDiagnosticListener::emit(Diagnostics::Diagnostic &diag) {
        diag.file = this->file;
        if (this->onDiagnostic != nullptr) {
            this->onDiagnostic(diag);
        }
    }

    void ANTLRDiagnosticListener::syntaxError(
        antlr4::Recognizer *recognizer,
        antlr4::Token *offendingSymbol,
        size_t line,
        size_t charPositionInLine,
        const std::string &msg,
        std::exception_ptr e
    ) {
        this->syntaxErrorsDetected = true;

        Diagnostics::Diagnostic diag = Diagnostics::antlrSyntaxErrorToDiagnostic(offendingSymbol, line, charPositionInLine, msg, e);
        this->emit(diag);
    }

    void ANTLRDiagnosticListener::reportAmbiguity(
        antlr4::Parser *recognizer,
        const antlr4::dfa::DFA &dfa,
        size_t startIndex,
        size_t stopIndex,
        bool exact,
        const antlrcpp::BitSet &ambigAlts,
        antlr4::atn::ATNConfigSet *configs
    ) {
        if (!this->reportAmbiguities) {
            return;
        }

        Diagnostics::Diagnostic diag = Diagnostics::antlrAmbiguityToDiagnostic(
            recognizer, dfa, startIndex, stopIndex, exact, ambigAlts, configs
        );
        this->emit(diag);
    }
}
