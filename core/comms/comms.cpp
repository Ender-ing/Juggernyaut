/**
 * @brief
 * Enable uniform reports for both consoles and IDEs
**/

#include "comms.hpp"

namespace Comms {
    // Communication mode
    // (may need to introduce more complex modes!)
    Mode mode = CLI_MODE; // defaults to CLI mode!

    // Handle error throw statement
    static void throwError(std::string msg) {
        std::cerr << CLI::color("[Internal Error] ", Comms::CLI::Color::red)
            << CLI::color(msg, Comms::CLI::Color::red)
            << CLI::color(BAD_CODE_OR_MEMORY_LEAKS,
                Comms::CLI::Color::red)
            << std::endl;
        throw std::runtime_error(msg);
    }

    // General compiler status
    namespace ProcessReport {
        // It may become necessary to introduce static tracking variables to keep up with more complex error reports
        // to support LSP!
        int programStatus = 0; // Default: 0 (success)
        // Keep track of the reporting status
        bool didSendReport = false;
    }

    // Report-specific status
    namespace IndividualReport {
        // Current reporting status!
        bool isNew = true; // Check if this is a new report!
        ReportType type = UNKNOWN_REPORT;
        std::string stage = "";
        std::stringstream messageStream; // report message data!

        // Code-related report data
        std::string path = "";
        size_t startLine = 0;
        size_t startColumn = 0;
        size_t endLine = 0;
        size_t endColumn = 0;

        // Reset report data!
        static void reset() {
            // Report basic data
            isNew = true;
            type = UNKNOWN_REPORT;
            stage = "";
            messageStream.str(""); // Clear the internal buffer
            messageStream.clear(); // Clear the state flags (eofbit, failbit, badbit)

            // Code-related data
            path = "";
            startLine = 0;
            startColumn = 0;
            endLine = 0;
            endColumn = 0;
        }

        // Send report data!
        static void send() {
            if (IndividualReport::isNew) {
                throwError("Attempting to send an untouched/incorrectly initialised Comms::IndividualReport!");
            }
            if(!ProcessReport::didSendReport){
                ProcessReport::didSendReport = true;
            }
            // Send report data to CLI/LSP
            if (mode == CLI_MODE) {
                // Print report details
                CLI::Reports::print();
            } else if (mode == LSP_MODE) {
                throwError("Currently, 'LSP' Comms::mode is not supported!");
            } else {
                throwError("Unsupported Comms::mode value!"); 
            }
            // Reset report data
            reset();
        }
    }

    // Keep track of general report statistics
    namespace Statistics {
        int normalReports = 0;
        int warningReports = 0;
        int criticalReports = 0;
        int fatalReports = 0;
        int actionReports = 0;
        int debugReports = 0;
    }

    // Internal library members!
    namespace ReportInternals {
        // Handle control input processing
        static void processReportControl(const ReportInput& arg) {
            // Arguments that are used to send instructions!
            if (std::holds_alternative<ReportAction>(arg)) {
                ReportAction value = std::get<ReportAction>(arg);
                if (value == START_REPORT){
                    if (!IndividualReport::isNew) {
                        throwError("Starting a new Comms::IndividualReport without ending the previous one!"); 
                    }
                    IndividualReport::isNew = false;
                } else if (value == END_REPORT) {
                    // Send report data
                    IndividualReport::send();
                } else {
                    throwError("Unknown Comms::ReportAction value!"); 
                }
            } else if (std::holds_alternative<ReportType>(arg)) {
                ReportType value = std::get<ReportType>(arg);

                // Track report type
                switch (value) {
                    case NORMAL_REPORT:
                        Statistics::normalReports++;
                        break;
                    case WARNING_REPORT:
                        Statistics::warningReports++;
                        break;
                    case CRITICAL_REPORT:
                        Statistics::criticalReports++;
                        // Update program status
                        ProcessReport::programStatus = 1;
                        break;
                    case FATAL_REPORT:
                        Statistics::fatalReports++;
                        // Update program status
                        ProcessReport::programStatus = 1;
                        break;
                    case ACTION_REPORT:
                        Statistics::actionReports++;
                        break;
                    case DEBUG_REPORT:
                        Statistics::debugReports++;
                        break;
                
                    default:
                        throwError("Unknown Comms::ReportType value!");
                }

                // Value is valid!
                IndividualReport::type = value;
            } else {
                throwError("Unknown Comms::report control argument type!");
            }
        }

        // Handle user input processing
        static void processReportInput(const ReportInput& arg) {
            // Check if the report is valid
            if (IndividualReport::isNew) {
                throwError("Attempting to inject a value to an uninitialised Comms::report!");
            }
            // Check if it's data that should saved, or printed!
            bool isMessageBody = true;
            std::stringstream text;
            // Check arg type
            if (std::holds_alternative<std::string>(arg)) {
                std::string value = std::get<std::string>(arg);
                // TMP
                text << value;
            } else if (std::holds_alternative<size_t>(arg)) {
                size_t value = std::get<size_t>(arg);
                // TMP
                text << value;
                // Check for line and column numbers!
                // ...
                // isMessageBody = false
            } else {
                throwError("Unknown Comms::report input argument type!"); 
            }

            // Save message body data
            if (isMessageBody) {
                IndividualReport::messageStream << text.str();
            }
        }
    }

    // Type group checks
    template <typename... Types, typename Variant>
    static bool holdsAlternatives(const Variant& variant) {
        return (std::holds_alternative<Types>(variant) || ...); // Fold expression
    }

    // Reporting
    void report(const ReportInputs& args) {
        for (const auto& arg : args) {
            if (holdsAlternatives<REPORT_BASIC_TYPES>(arg)) {
                ReportInternals::processReportControl(arg);
            } else if (holdsAlternatives<REPORT_INPUT_TYPES>(arg)) {
                ReportInternals::processReportInput(arg);
            } else {
                throwError("Unknown Comms::report argument type!"); 
            }
        }
    }

    // Initalise protocol
    void initalize() {
        if(mode == CLI_MODE){
            // Initialise CLI mode
            CLI::initialize(); //TMP
        } else if(mode == LSP_MODE){
            // Initialise LSP mode
            // ...
        } else {
            throwError("Unknown Comms::mode value!"); 
        }
    }

    // Keep track of finalisation
    static bool isFinalized = false;
    // For actions that require minimal finalisation!
    bool minimalProtocolFinalization = false;

    // Finalise protocol
    void finalize() {
        // Check for unwanted called
        if (isFinalized) {
            throwError("Detecting multiple protocol finalisation attempts!");
        }
        isFinalized = true;

        if(mode == CLI_MODE){
            // Finalize CLI mode
            CLI::finalize(); //TMP
        } else if(mode == LSP_MODE){
            // Finalize LSP mode
            // ...
        } else {
            throwError("Unknown Comms::mode value!"); 
        }

        // Check for unfinished reports
        if(ProcessReport::didSendReport && !IndividualReport::isNew){
            throwError("Detected an unfinished report!");
        }
    }
}
