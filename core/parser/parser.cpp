/**
 * @brief
 * Parser implementations
**/

// ANTLR4 imports
#include "antlr4-runtime.h"
#include "JuggernyautLexer.h"
#include "JuggernyautParser.h"

// Parser
#include "parser.hpp"
#include "listeners/errors.hpp"

// Comms
#include "../comms/comms.hpp"

namespace Parser {
    namespace Debug {
        // Check for syntax errors
        // [true -> success, false -> failure]
        bool syntaxCheck (std::string file_contents) {
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

            // Print tokens
            tokens.fill();
            REPORT(Comms::START_REPORT, Comms::DEBUG_REPORT, "Tokens: \n");
            for (auto token : tokens.getTokens()) {
                REPORT(token->toString(), "\n");
            }
            REPORT(Comms::END_REPORT);

            // Get the start tree!
            antlr4::tree::ParseTree *tree = parser.program();

            // Print the parse tree!
            auto s = tree->toStringTree(&parser);
            REPORT(Comms::START_REPORT, Comms::DEBUG_REPORT, "Parse Tree: \n", s, Comms::END_REPORT);

            return Listeners::errorsDetected;
        }
    }
}
