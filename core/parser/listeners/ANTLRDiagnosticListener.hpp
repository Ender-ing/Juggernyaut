/**
 * @brief
 * ANTLR4 Lexer & Parser diagnostic listener
**/
#pragma once
#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// ANTLR4 imports
#include "../antlr4.hpp"

// Diagnostics
#include "../../diagnostics/Diagnostic.hpp"

// Environment
#include "../../data/driver/Environment.hpp"

namespace Parser::Listeners {
    class JUG_PARSER_API ANTLRDiagnosticListener final : public antlr4::BaseErrorListener {
        private:
            const Data::Host::File *file = nullptr;
            const Diagnostics::Event &onDiagnostic;
            bool reportAmbiguities = false;

            bool syntaxErrorsDetected = false;
            void emit(Diagnostics::Diagnostic &diag) ;
        public:
            ANTLRDiagnosticListener(const Data::Host::File *targetFile, const Diagnostics::Event &diagnosticCallback, bool enableAmbiguityReports = false)
                : file(targetFile), onDiagnostic(diagnosticCallback), reportAmbiguities(enableAmbiguityReports) {}

            ANTLRDiagnosticListener(const ANTLRDiagnosticListener&) = delete;
            ANTLRDiagnosticListener& operator=(const ANTLRDiagnosticListener&) = delete;

            bool hasErrors() const noexcept { return this->syntaxErrorsDetected; }

            // ANTLR4 BaseErrorListener overrides
            void syntaxError(
                antlr4::Recognizer *recognizer,
                antlr4::Token *offendingSymbol,
                size_t line,
                size_t charPositionInLine,
                const std::string &msg,
                std::exception_ptr e
            ) override;
            void reportAmbiguity(
                antlr4::Parser *recognizer,
                const antlr4::dfa::DFA &dfa,
                size_t startIndex,
                size_t stopIndex,
                bool exact,
                const antlrcpp::BitSet &ambigAlts,
                antlr4::atn::ATNConfigSet *configs
            ) override;
    };
}
