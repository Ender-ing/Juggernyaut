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
#include "store/Documents.hpp"

// Front-End Driver
#include "../core/driver/JugDriver.hpp"

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
        Console::initalize({
            "Juggernyaut Compiler",
            Base::Info::version,
            Base::Info::build,
            Base::Info::arch,
            Base::Info::osName,
            Base::Info::osVersion
        });
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

    // Set up the driver
    auto docs = std::make_unique<Store::Documents>();
    Driver::JugDriver::HostConfigs host;
    host.vfs = docs.get();
    Driver::JugDriver::DriverConfigs configs;
    Driver::JugDriver::JugDriver driver(host, configs);

    // Handle diagnostics
    driver.configs.hooks.driver.onDiagnostic = [&driver](const Diagnostics::Diagnostic &diag){
        if (diag.file != nullptr) {
            // Get the position
            Console::IndividualReport::path = diag.file->canonical;
            Console::IndividualReport::startLine = diag.range.start.line;
            Console::IndividualReport::startColumn = diag.range.start.character;
            Console::IndividualReport::endLine = diag.range.end.line;
            Console::IndividualReport::endColumn = diag.range.end.character;
        }

        Console::IndividualReport::code = diag.code;

        // Update stage data
        int stageId = static_cast<int>(std::floor(diag.code / 100000));
        if (stageId == 1) {
            Console::IndividualReport::stage = "Lexer";
        } else if (stageId == 2) {
            Console::IndividualReport::stage = "Parser";
        } else if (stageId == 7) {
            Console::IndividualReport::stage = "Driver";
        } else {
            Console::IndividualReport::stage = "?Unknown Stage?";
        }
        REPORT(Console::START_REPORT, getReportType(diag.severity), diag.message, Console::END_REPORT);
    };

    // Stats
    uint32_t sourceVisits = 0;

    // Add input files
    if (Base::InitialConfigs::Input::entryPaths.empty()) {
        const std::string configUri = driver.lookupProject(driver.host.vfs->cwd());
        if (configUri == "") {
            REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "couldn't find a 'jug.toml' configuration file!", Console::END_REPORT);

            return 1;
        }
        driver.addProject(configUri);
    }else {
        driver.processPaths(Base::InitialConfigs::Input::entryPaths);
    }
    // sourceVisits; // THIS IS OUT OF SYNC! 

    // Debug action
    /*if (Base::InitialConfigs::Debug::Parser::activateAntlrSyntaxTest) {
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
    }*/

    // Check for requested termination
    if (Base::InitialConfigs::Technical::terminateAfterActions) {
        // End the program
        Console::finalize();
        return Console::ProcessReport::programStatus;
    }

    /*session.hooks.parser.onContextStart = [&session, &sourceVisits](const Data::Store::SourceId srcId) {
        std::unique_ptr<Data::Store::Source> &source = (session.store)->getSourceById(srcId);

        REPORT(Console::START_REPORT, Console::NORMAL_REPORT, "\n#", ++sourceVisits, ": ", source->uri, Console::END_REPORT);
    };

    // Parser Diagnostics
    session.hooks.parser.onContextEnd = [&session](const Data::Store::SourceId srcId) {
        std::unique_ptr<Data::Store::Source> &source = (session.store)->getSourceById(srcId);
        const std::string &srcPath = source->uri;

        source->visitParserDiagnostics([&srcPath](const Diagnostics::Diagnostic &diag) {
            // Get the position
            Console::IndividualReport::path = srcPath;
            Console::IndividualReport::startLine = diag.range.start.line;
            Console::IndividualReport::startColumn = diag.range.start.character;
            Console::IndividualReport::endLine = diag.range.end.line;
            Console::IndividualReport::endColumn = diag.range.end.character;

            Console::IndividualReport::code = diag.code;

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
    };*/

    // Begin the actual work here...
    driver.run();
    
    // End the program
    Console::finalize(sourceVisits);
    return Console::ProcessReport::programStatus;
}
