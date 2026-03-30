/**
 * @brief
 * Lexer class
**/

#include "JugLexer.hpp"

namespace Parser {
    namespace Internal {
        // Handle "error tokens" (those that start with "INVALID")
        void JugLexer::emit(std::unique_ptr<antlr4::Token> curToken) {
            // Intercept the token before it's finalized
            size_t tokenType = curToken->getType();
            bool shouldDispatch = false;
            std::string msg;

            switch (tokenType) {
            case JuggernyautLexer::INVALID_1_LIT_STRING:
                msg = "9"; // LEXER ERROR CODE!
                shouldDispatch = true;
                break;
            case JuggernyautLexer::INVALID_2_LIT_STRING:
                msg = "10"; // LEXER ERROR CODE!
                shouldDispatch = true;
                break;
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
