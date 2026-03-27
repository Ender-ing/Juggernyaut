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

#include "internal/JugLexer.hpp"

namespace Parser {
    namespace Debug {
        
        // Check for syntax errors
        // [true -> success, false -> failure]
        bool syntaxCheck(const std::string &file_contents, const TokenReport &onTokenCall, const TreeReport &onTreeCall,
            Listeners::ErrorListener *lexerErrorListener, Listeners::ErrorListener *parserErrorListener) {
            // Use the file's input
            antlr4::ANTLRInputStream input(file_contents);

            // Tokens
            Internal::JugLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);
  
            // Parse tree
            GeneratedParser::JuggernyautParser parser(&tokens);

            // Check for syntax errors
            lexer.removeErrorListeners();// remove default parser error listeners.
            lexer.addErrorListener(lexerErrorListener);
            parser.removeErrorListeners();// remove default parser error listeners.
            parser.addErrorListener(parserErrorListener);

            // Pass tokens
            tokens.fill();
            for (auto token : tokens.getTokens()) {
                onTokenCall((const std::string) token->toString());
            }

            // Get the start tree!
            antlr4::tree::ParseTree *tree = parser.prog();

            // Pass the parse tree!
            onTreeCall((const std::string) tree->toStringTree(&parser));

            return Listeners::errorsDetected;
        }
    }
}
