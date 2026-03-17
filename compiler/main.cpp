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

        // Debug (TMP)
        // Listen for syntax-related errors
        class DebugErrorListener : public Parser::Listeners::ErrorListener {
            private:
                // (Storing a std::string value will result in a C4251 MSVC warning)
                const char* stage; // "Lexer" or "Parser"
            public:
                // Constructors
                DebugErrorListener(const char* stageName) : stage(stageName) { }
                DebugErrorListener(const std::string& stageName) : stage(stageName.c_str()) {}

                // ANTLR4 functions
                void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
                    size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {
                    REPORT(Console::END_REPORT);

                    // Get the starting position
                    Console::IndividualReport::startLine = line;
                    Console::IndividualReport::startColumn = charPositionInLine;

                    // Determine the end position
                    std::string tokenText;
                    if (offendingSymbol) {
                        // TMP
                        tokenText = offendingSymbol->getText();
                        Console::IndividualReport::endColumn = Console::IndividualReport::startColumn + tokenText.length();
                    } else {
                        // TMP
                        tokenText = "<N/A>";
                        Console::IndividualReport::endColumn = Console::IndividualReport::startColumn + 1;
                    }
                    Console::IndividualReport::endLine = line; // TMP

                    // Update stage data
                    Console::IndividualReport::stage = this->stage;

                    // Report the error
                    REPORT(Console::START_REPORT, Console::CRITICAL_REPORT,
                        msg, "(Token Text: '", tokenText, "')",
                        Console::END_REPORT);

                    REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "\n");
                }
                void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
                    size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs)
                    override {

                    REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Ambiguity reported from index ",
                        startIndex ," to index " , stopIndex, Console::END_REPORT);
                }
                void reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
                    antlr4::atn::ATNConfigSet *configs) override {

                    REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Attempting full context reported from index ",
                        startIndex ," to index " , stopIndex, Console::END_REPORT);
                }
                void reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
                    size_t startIndex, size_t stopIndex, size_t prediction,
                    antlr4::atn::ATNConfigSet *configs) override {

                    REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Context sensitivity reported from index ",
                        startIndex ," to index " , stopIndex, Console::END_REPORT);
                }
        };

        DebugErrorListener lexerDebugErrorListener("Lexer");
        DebugErrorListener parserDebugErrorListener("Parser");

        REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Tokens: \n");
        Parser::Debug::syntaxCheck(file_contents,
            [](const std::string tokenText){
                REPORT(tokenText, "\n");
            },
            [](const std::string treeText){
                REPORT(Console::END_REPORT);
                REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Parse Tree: \n", treeText, Console::END_REPORT);
            }, &lexerDebugErrorListener, &parserDebugErrorListener);
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
