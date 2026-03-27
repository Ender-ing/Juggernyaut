/**
 * @brief
 * Manage diagnostic code generation
**/

// ANTLR runtime
#include "antlr4-runtime.h"
#include "DiagnosticsCodeLexer.h"
#include "DiagnosticsCodeParser.h"

using namespace antlr4;

int main(int argc, const char* argv[]) {
    // 1. Setup the input. Using a hardcoded string to test your error code syntax quickly.
    // You can swap this with ANTLRFileStream to read from a file instead.
    std::string testInput = "0[1, \"test\"]:\n-Oh noooo!\n";
    ANTLRInputStream input(testInput);

    // Lexer: Convert the input character stream into a token stream
    GeneratedLexer::DiagnosticsCodeLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Print
    tokens.fill();
    std::cout << "========== TOKENS ==========\n";
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }
    std::cout << "============================\n\n";
    GeneratedParser::DiagnosticsCodeParser parser(&tokens);
    tree::ParseTree *tree = parser.list();
    std::cout << "======== PARSE TREE ========\n";
    std::cout << tree->toStringTree(&parser) << std::endl;
    std::cout << "============================\n";

    return 0;
}
