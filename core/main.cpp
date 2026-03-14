/**
 * @brief
 * Juggernyaut Compiler
**/

// Basic C++ headers
#include <fstream>

// Common headers
#include "common/headers.hpp"
#include "common/debug.hpp"

// CLI/LSP
#include "comms/comms.hpp"

// Parser
#include "parser/parser.hpp"

// Base
#include "base.config.hpp"
#include "base.info.hpp"

int main (int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    // Initalise communications protocol
    // (Basiclly allowing the default protocol to take effect)
    if (!Base::InitialConfigs::Technical::shouldSkipDefaultInitialization(argc, argv)) {
        // This is done to allow flags like --version to function normally
        Comms::initalize();
    }

    // Update initial configurations
    if(!Base::InitialConfigs::updateUsingArgs(argc, argv)){
        // This process failed!
        if (Comms::Statistics::fatalReports == 0) {
            REPORT(Comms::START_REPORT, Comms::FATAL_REPORT, "Terminating program due to a Base::InitialConfigs error!",
                BAD_CODE_OR_MEMORY_LEAKS,
                Comms::END_REPORT);
        }

        // End the program
        Comms::finalize();
        return Comms::ProcessReport::programStatus;
    }

    // Check if other delayed actions are allowed to run
    if (Base::InitialConfigs::Technical::terminateAfterArgs) {
        // End the program
        Comms::finalize();
        return Comms::ProcessReport::programStatus;
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
            REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT, "Error opening file: ", filename, Comms::END_REPORT);
            // End the program
            Comms::finalize();
            return Comms::ProcessReport::programStatus;
        }
        // Debug
        Parser::Debug::syntaxCheck(file_contents);
    }

    // Check for requested termination
    if (Base::InitialConfigs::Technical::terminateAfterActions) {
        // End the program
        Comms::finalize();
        return Comms::ProcessReport::programStatus;
    }

    // Begin the actual work here...
    
    // Handle memory check results
    if(Common::CrtDebug::processCrtMemoryReports()){
        // Exist with an error on memory leaks!
        REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT,
            "Terminating program due to detected memory errors! Please contact the developers of Juggernyaut!",
            Comms::END_REPORT);
        return 1;
    }

    // End the program
    Comms::finalize();
    return Comms::ProcessReport::programStatus;
}
