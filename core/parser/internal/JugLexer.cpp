/**
 * @brief
 * Lexer class
**/

#include "JugLexer.hpp"

namespace Parser {
    namespace Internal {
        // Internal
        void internal_dispatchSyntaxError(JugLexer *lexer, std::unique_ptr<antlr4::Token> &curToken, const std::string &msg) {
            lexer->getErrorListenerDispatch().syntaxError(
                lexer, // The recognizer
                curToken.get(), // Offending symbol (null for lexer)
                curToken->getLine(), // Line from the token
                curToken->getCharPositionInLine(), // Column from the token
                std::move(msg), // Your custom message
                nullptr // Exception pointer
            );
        }

        // Handle "error tokens" (those that start with "INVALID")
        void JugLexer::emit(std::unique_ptr<antlr4::Token> curToken) {
            // Intercept the token before it's finalized
            size_t tokenType = curToken->getType();

            switch (tokenType) {
            case JuggernyautLexer::INVALID_1_LIT_STRING:
                internal_dispatchSyntaxError(this, curToken, "100002"); // LEXER ERROR CODE!
                break;
            case JuggernyautLexer::INVALID_2_LIT_STRING:
                internal_dispatchSyntaxError(this, curToken, "100003"); // LEXER ERROR CODE!
                break;
            default:
                break;
            }

            // Pass it along to the standard system
            JuggernyautLexer::emit(std::move(curToken));
        }
    }
}
