/**
 * @brief
 * Lexer class
**/

#include "JugLexer.hpp"

namespace Parser {
    namespace Internal {
        void JugLexer::emit(std::unique_ptr<antlr4::Token> curToken) {
            // Intercept the token before it's finalized
            size_t tokenType = curToken->getType();
            bool shouldDispatch = false;
            std::string msg;

            switch (tokenType) {
            default:
                break;
            }

            if (shouldDispatch) {
                this->getErrorListenerDispatch().syntaxError(
                    this, // The recognizer
                    curToken.get(), // Offending symbol (null for lexer)
                    curToken->getLine(), // Line from the token
                    curToken->getCharPositionInLine(), // Column from the token
                    std::move(msg), // Your custom message
                    nullptr // Exception pointer
                );
            }

            // Pass it along to the standard system
            JuggernyautLexer::emit(std::move(curToken));
        }
    }
}
