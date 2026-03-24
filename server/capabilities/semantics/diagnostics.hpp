/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

// Parser
#include "../../../core/parser/parser.hpp"
#include "../../../core/parser/listeners/errors.hpp"

// Store
#include "../../store/DocumentStore.hpp"

namespace Capabilities {
    namespace Semantics {
        // TMP
        // Listen for syntax-related errors
        class SyntaxErrorListener : public Parser::Listeners::ErrorListener {
            private:
                // (Storing a std::string value will result in a C4251 MSVC warning)
                const char *stage; // "Lexer" or "Parser"
                std::vector<lsp::Diagnostic> &diags;
            public:
                // Constructors
                SyntaxErrorListener(const char *stageName, std::vector<lsp::Diagnostic> &diagsVector) : stage(stageName), diags(diagsVector) { }
                SyntaxErrorListener(const std::string &stageName, std::vector<lsp::Diagnostic> &diagsVector) : stage(stageName.c_str()), diags(diagsVector) {}

                // ANTLR4 functions
                virtual void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override ;
                virtual void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
                    size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs)
                    override ;
                virtual void reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
                    antlr4::atn::ATNConfigSet *configs) override ;
                virtual void reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, size_t prediction,
                    antlr4::atn::ATNConfigSet *configs) override ;
        };

        extern void validateDocumentSyntax(lsp::MessageHandler &messageHandler, Store::Document &doc) ;
    }
}
