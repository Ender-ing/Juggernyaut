/**
 * @brief
 * Juggernyaut Compiler
**/

// Basic C++ headers
#include <fstream>

// Common headers
#include "../core/common/headers.hpp"
#include "../core/common/debug.hpp"

#include "console/console.hpp"

// Parser
#include "../core/parser/parser.hpp"

// Base
#include "base/config.hpp"
#include "base/info.hpp"

int main (int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    // Initalise communications protocol
    // (Basiclly allowing the default protocol to take effect)
    if (!Base::InitialConfigs::Technical::shouldSkipDefaultInitialization(argc, argv)) {
        // This is done to allow flags like --version to function normally
        Console::initalize();
    }

    // Update initial configurations
    if(!Base::InitialConfigs::updateUsingArgs(argc, argv)){
        // This process failed!
        if (Console::Statistics::fatalReports == 0) {
            REPORT(Console::START_REPORT, Console::FATAL_REPORT, "Terminating program due to a Base::InitialConfigs error!",
                BAD_CODE_OR_MEMORY_LEAKS,
                Console::END_REPORT);
        }

        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    // Check if other delayed actions are allowed to run
    if (Base::InitialConfigs::Technical::terminateAfterArgs) {
        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    // Now run delayed actions

    // TMP
    if (Base::InitialConfigs::Debug::Parser::activateAntlrSyntaxTest) {
        auto filename = Base::InitialConfigs::mainPath;
        // Check first input argument path
        std::ifstream file(filename);

        // Check if the file actually exists and get its contents
        std::string file_contents;
        if (file.is_open()) {
            // Get all file stream
            std::stringstream buffer;
            buffer << file.rdbuf();

            // Get the file as a string
            file_contents = buffer.str();

            // Close file
            file.close();
        } else {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Error opening file: ", filename, Console::END_REPORT);
            // End the program
            Console::finalize();
            return Console::ProcessReport::programStatus;
        }
        // Debug
        Parser::Debug::syntaxCheck(file_contents);
        REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Tokens: \n");
        Parser::Debug::syntaxCheck(file_contents,
            [](const std::string tokenText){
                REPORT(tokenText, "\n");
            },
            [](const std::string treeText){
                REPORT(Console::END_REPORT);
                REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Parse Tree: \n", treeText, Console::END_REPORT);
            }, lexerDebugErrorListener, parserDebugErrorListener);
    }

    // Check for requested termination
    if (Base::InitialConfigs::Technical::terminateAfterActions) {
        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    // Begin the actual work here...
    
    // Handle memory check results
    if(Common::CrtDebug::processCrtMemoryReports()){
        // Exist with an error on memory leaks!
        REPORT(Console::START_REPORT, Console::CRITICAL_REPORT,
            "Terminating program due to detected memory errors! Please contact the developers of Juggernyaut!",
            Console::END_REPORT);
        return 1;
    }

    // End the program
    Console::finalize();
    return Console::ProcessReport::programStatus;
}
