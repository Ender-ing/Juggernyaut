/**
 * @brief
 * Include all used comms headers
**/

#pragma once

#include "../../core/common/headers.hpp"

// Basic C++ headers
#include <variant>

#include "basic.hpp"
#include "report.hpp"
#include "initialize.hpp"
#include "finalize.hpp"
#include "optimization.hpp"

// Pack the report function into a macro
#define REPORT(...)                                                 \
    Console::report(Console::ReportInputs{__VA_ARGS__})             \

namespace Console {
    // General compiler status
    namespace ProcessReport {
        // It may become necessary to introduce static tracking variables to keep up with more complex error reports
        // to support LSP!
        extern int programStatus;
        // Keep track of the reporting status
        extern bool didSendReport;
    }

    // Report actions
    enum ReportAction {
        START_REPORT = 0xFFF00101, // Used as a measure to prevent the misuse of the report function
        END_REPORT = 0xFFF00102 // Used to send and reset report data!
    };
    // Report types
    enum ReportType {
        UNKNOWN_REPORT = 0xFFF00200, // Placeholder for unknown report types
        NORMAL_REPORT = 0xFFF00201, // Used for general reports
        WARNING_REPORT = 0xFFF00202, // Used for warnings
        CRITICAL_REPORT = 0xFFF00203, // Used for errors
        FATAL_REPORT = 0xFFF00204, // Used for fatal errors (terminates the program)
        ACTION_REPORT = 0xFFF00205, // Used for user-invoked action confirmations
        DEBUG_REPORT = 0xFFF00206 // Used for debug/non-standard output
    };

    // Handle shared input types
    #define REPORT_BASIC_TYPES ReportType, ReportAction
    #define REPORT_INPUT_TYPES size_t, std::string
    typedef std::variant<REPORT_BASIC_TYPES, REPORT_INPUT_TYPES> ReportInput;
    typedef std::vector<ReportInput> ReportInputs;

    // Report-specific status
    namespace IndividualReport {
        // Current reporting status!
        extern bool isNew; // Check if this is a new report!
        extern ReportType type;
        extern std::string stage;
        extern std::stringstream messageStream;

        extern bool isContinuation;

        // Code-related report data
        extern std::string path;
        extern size_t startLine;
        extern size_t startColumn;
        extern size_t endLine;
        extern size_t endColumn;
    }

    // Keep track of general report statistics
    namespace Statistics {
        extern int normalReports;
        extern int warningReports;
        extern int criticalReports;
        extern int fatalReports;
        extern int actionReports;
        extern int debugReports;
    }

    // Reporting
    extern void report(const ReportInputs& args) ;

    extern void runtimeTracking() ;

    // Initalise protocol
    extern void initalize() ;

    // For actions that require minimal finalisation!
    extern bool minimalProtocolFinalization;

    // Finalise protocol
    extern void finalize() ;
}
