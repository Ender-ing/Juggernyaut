/**
 * @brief
 * ANTLR4 cleanup
**/

#include "cleanup.hpp"

// ANTLR4 imports
#include "../antlr4.hpp"
#include "atn/LexerATNSimulator.h"
#include "atn/ParserATNSimulator.h"

#include "JugLexer.hpp"
#include "JugParser.hpp"

namespace Parser {
    namespace Internal {
        void flushAntlrCache() {
            antlr4::ANTLRInputStream emptyInput("");
            JugLexer tempLexer(&emptyInput);
            antlr4::CommonTokenStream emptyTokenStream(&tempLexer);
            JugParser tempParser(&emptyTokenStream);

            // Wipe shared Lexer simulator cache
            auto lexerSimulator = tempLexer.getInterpreter<antlr4::atn::LexerATNSimulator>();
            if (lexerSimulator != nullptr) {
                lexerSimulator->clearDFA();
            }
            // Wipe shared Parser simulator cache
            auto parserSimulator = tempParser.getInterpreter<antlr4::atn::ParserATNSimulator>();
            if (parserSimulator != nullptr) {
                parserSimulator->clearDFA();
            }
        }
    }
}
