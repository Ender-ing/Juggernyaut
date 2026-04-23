/**
 * @brief
 * Juggernyaut Compiler
**/

// Basic C++ headers
#include <fstream>

// Common headers
#include "common/headers.hpp"

#include "console/console.hpp"

// Compiler Store
#include "store/FileStore.hpp"

// Session
#include "../core/session/session.hpp"

// Configs
#include "../configs/configs.hpp"

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

    // Initalise communications protocol
    // (Basiclly allowing the default protocol to take effect)
    if (!Base::InitialConfigs::Technical::shouldSkipDefaultInitialization(argc, argv)) {
        // This is done to allow flags like --version to function normally
        Console::initalize();
    }

    // Update initial configurations
    if(!Base::InitialConfigs::updateUsingArgs(argc, argv)){
        // This process failed!
        if (Console::Statistics::fatalReports != 0) {
            REPORT(Console::START_REPORT, Console::FATAL_REPORT, "terminating program due to a Base::InitialConfigs error!",
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

    // Setup session
    Store::FileStore store = Store::FileStore();
    Session::Session session = Session::getSessionDefaults();
    session.store = &store;

    // Load external configs
    if (!Base::InitialConfigs::Input::config.empty()) {
        std::string errorLog;
        if (!Configs::modifySession(session, Base::InitialConfigs::Input::config, errorLog, false)) {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "couldn't process configuration file: ",
                errorLog, Console::END_REPORT);
        }
        errorLog.clear();
        errorLog.shrink_to_fit();
    }

    // Add import directories
    for (const auto &dir : Base::InitialConfigs::Input::importDirs) {
        store.addImportDir(dir);
    }

    // Add input files
    std::string currentPath;
    for (const auto &path : Base::InitialConfigs::Input::entryPaths) {
        if (store.resolvePath(path, currentPath)) {
            store.addSource(currentPath, true);
        } else {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "couldn't resolve input path: ", store._getCanonical(path), Console::END_REPORT);
        }
    }
    currentPath.clear();
    currentPath.shrink_to_fit();

    // Debug action
    if (Base::InitialConfigs::Debug::Parser::activateAntlrSyntaxTest) {
        session.hooks.parser.onANTLRTokenDetected = [](const uint8_t &stage, const std::string &tokenText) {
            if (stage == 2) {
                REPORT(tokenText, "\n");
            } else if (stage == 1) {
                REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "tokens: \n");
            } else if (stage == 3) {
                REPORT(Console::END_REPORT);
            }
        };
        session.hooks.parser.onANTLRTreeGenerated = [](const std::string &treeText) {
            REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "parse tree: \n", treeText, Console::END_REPORT);
        };
    }

    // Check for requested termination
    if (Base::InitialConfigs::Technical::terminateAfterActions) {
        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    uint32_t activeSources = 0;
    session.hooks.parser.onContextStart = [&session, &activeSources](const Data::Store::SourceId srcId) {
        std::unique_ptr<Data::Store::Source> &source = (session.store)->getSourceById(srcId);

        REPORT(Console::START_REPORT, Console::NORMAL_REPORT, "#", ++activeSources, ": ", source->uri, Console::END_REPORT);
    };

    // Parser Diagnostics
    session.hooks.parser.onContextEnd = [&session](const Data::Store::SourceId srcId) {
        std::unique_ptr<Data::Store::Source> &source = (session.store)->getSourceById(srcId);

        source->visitParserDiagnostics([](const Diagnostics::Diagnostic &diag) {
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
            REPORT(Console::START_REPORT, getReportType(diag.severity), diag.message, Console::END_REPORT);
        });
    };

    // Begin the actual work here...
    Session::initiate(session);
    
    // End the program
    Console::finalize(activeSources);
    return Console::ProcessReport::programStatus;
}
