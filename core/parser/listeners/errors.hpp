/**
 * @brief
 * Lexer & parser error listeners
**/

#pragma once


// ANTLR4 imports
namespace antlr4 {
    class BaseErrorListener;
    class Recognizer;
    class Token;
    class Parser;
    namespace dfa { class DFA;}
    namespace atn { class atn::ATNConfigSet; }
}
namespace antlrcpp { class BitSet; }

#include "../../common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// Basic C++ headers
#include <memory>

namespace Parser {
    namespace Listeners {
        // Track errors
        extern JUG_PARSER_API bool errorsDetected;

        // Listen for syntax-related errors
        class JUG_PARSER_API ErrorListener : public antlr4::BaseErrorListener {
            private:
                // (Storing a std::string value will result in a C4251 MSVC warning)
                const char* stage; // "Lexer" or "Parser"
            public:
                // Constructors
                ErrorListener(const char* stageName) : stage(stageName) {}
                ErrorListener(const std::string& stageName) : stage(stageName.c_str()) {}

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
        };
    }
}
