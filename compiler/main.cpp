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

// Compiler Store
#include "store/FileStore.hpp"

// Session
#include "../core/session/session.hpp"

// Base
#include "base/config.hpp"
#include "base/info.hpp"

const Console::ReportType getReportType(Diagnostics::Severity severity) {
    switch (severity) {
    case Diagnostics::Severity::Error:
        return Console::CRITICAL_REPORT;
    case Diagnostics::Severity::Warning:
        return Console::WARNING_REPORT;
    case Diagnostics::Severity::Info:
        return Console::NORMAL_REPORT;
    case Diagnostics::Severity::Hint:
        return Console::NORMAL_REPORT;
    default:
        return Console::UNKNOWN_REPORT;
    }
}

int main(int argc, const char *argv[]) {
    Console::runtimeTracking();

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

    // Manage input files
    Store::FileStore store = Store::FileStore();

    // Add input files
    for (auto path : Base::InitialConfigs::entryPaths) {
        store.addSource(path, true);
    }

    // Setup session
    Session::Session session = Session::getSessionDefaults();
    session.store = &store;

    // Debug action
    if (Base::InitialConfigs::Debug::Parser::activateAntlrSyntaxTest) {
        session.hooks.parser.onContextStart = [](const Data::Store::SourceId srcId) {
            REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Tokens:", Console::END_REPORT);
        };
        session.hooks.parser.onANTLRTokenDetected = [](const std::string &tokenText) {
            Console::IndividualReport::isContinuation = true;
            REPORT(Console::START_REPORT, Console::DEBUG_REPORT, tokenText, Console::END_REPORT);
        };
        session.hooks.parser.onANTLRTreeGenerated = [](const std::string &treeText) {
            REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Parse Tree: \n", treeText, Console::END_REPORT);
        };
    }

    // Check for requested termination
    if (Base::InitialConfigs::Technical::terminateAfterActions) {
        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    // Parser Diagnostics (OLD! SWITCH TO FILE-BASED REPORTING)
    session.hooks.parser.onSyntaxError = [](const Diagnostics::Diagnostic &diag) {
        // Get the position
        Console::IndividualReport::startLine = diag.range.start.line;
        Console::IndividualReport::startColumn = diag.range.start.character;
        Console::IndividualReport::endLine = diag.range.end.line;
        Console::IndividualReport::endColumn = diag.range.end.character;

        // Update stage data
        int stageId = static_cast<int>(std::floor(diag.code / 100000));
        if (stageId == 1) {
            Console::IndividualReport::stage = "Lexer";
        } else if (stageId == 2) {
            Console::IndividualReport::stage = "Parser";
        } else {
            Console::IndividualReport::stage = "?Unknown Stage?";
        }

        // Report the error
        REPORT(Console::START_REPORT, getReportType(diag.severity), diag.message, Console::END_REPORT);
    };
    session.hooks.parser.onAmbiguity = [](const Diagnostics::Diagnostic &diag) {
        REPORT(Console::START_REPORT, getReportType(diag.severity), diag.message, " (source: ",
            diag.range.start.line, ":", diag.range.start.character ," to  " ,
            diag.range.end.line, ":", diag.range.end.character, ")", Console::END_REPORT);
    };

    // Begin the actual work here...
    Session::initiate(session);
    
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
