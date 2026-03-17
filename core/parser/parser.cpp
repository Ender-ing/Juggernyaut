/**
 * @brief
 * Parser implementations
**/

// Parser
#include "parser.hpp"

// ANTLR4 imports
#include "antlr4.hpp"
#include "JuggernyautLexer.h"
#include "JuggernyautParser.h"

#include "listeners/errors.hpp"

namespace Parser {
    namespace Debug {
        
        // Check for syntax errors
        // [true -> success, false -> failure]
        bool syntaxCheck (std::string file_contents, TokenReport onTokenCall, TreeReport onTreeCall) {
            // Use the file's input
            antlr4::ANTLRInputStream input(file_contents);

            // Tokens
            GeneratedLexer::JuggernyautLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);
  
            // Parse tree
            GeneratedParser::JuggernyautParser parser(&tokens);

            // Check for syntax errors
            Listeners::ErrorListener lexerErrorListener("Lexer");
            Listeners::ErrorListener parserErrorListener("Parser");
            lexer.removeErrorListeners();// remove default parser error listeners.
            lexer.addErrorListener(&lexerErrorListener);
            parser.removeErrorListeners();// remove default parser error listeners.
            parser.addErrorListener(&parserErrorListener);

            // Pass tokens
            tokens.fill();
            for (auto token : tokens.getTokens()) {
                onTokenCall((const std::string) token->toString());
            }

            // Get the start tree!
            antlr4::tree::ParseTree *tree = parser.program();

            // Pass the parse tree!
            onTreeCall((const std::string) tree->toStringTree(&parser));

            return Listeners::errorsDetected;
        }
    }
}
