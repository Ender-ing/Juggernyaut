/**
 * @brief
 * Juggernyaut Compiler
**/

// Basic C++ headers
#include <fstream>

// Common headers
#include "../core/common/headers.hpp"
#include "../core/common/debug.hpp"

// Parser
#include "../core/parser/parser.hpp"

int main (int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    // Initalise communications protocol
    // ...

    // End the program
    return 0;
}
