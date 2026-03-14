/**
 * @brief
 * Lexer & parser error listeners
**/

#include "errors.hpp"

#include "../../comms/comms.hpp"

namespace Parser {
    namespace Listeners {
        // Track errors
        bool errorsDetected = false;

        // Listen for syntax-related errors
        void ErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
            size_t charPositionInLine, const std::string &msg, std::exception_ptr e) {

            // Get the starting position
            Comms::IndividualReport::startLine = line;
            Comms::IndividualReport::startColumn = charPositionInLine;

            // Determine the end position
            std::string tokenText;
            if (offendingSymbol) {
                // TMP
                tokenText = offendingSymbol->getText();
                Comms::IndividualReport::endColumn = Comms::IndividualReport::startColumn + tokenText.length();
            } else {
                // TMP
                tokenText = "<N/A>";
                Comms::IndividualReport::endColumn = Comms::IndividualReport::startColumn + 1;
            }
            Comms::IndividualReport::endLine = line; // TMP

            // Update stage data
            Comms::IndividualReport::stage = this->stage;

            // Report the error
            REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT,
                msg, "(Token Text: '", tokenText, "')",
                Comms::END_REPORT);
            errorsDetected = true;
        }

        // TMP
        void ErrorListener::reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs) {

            REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT, "Ambiguity reported from index ",
                startIndex ," to index " , stopIndex, Comms::END_REPORT);
            errorsDetected = true;
        }
        void ErrorListener::reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, const antlrcpp::BitSet &conflictingAlts, antlr4::atn::ATNConfigSet *configs) {

            // TMP
            REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT, "Attempting full context reported from index ",
                startIndex ," to index " , stopIndex, Comms::END_REPORT);
            errorsDetected = true;
        }
        void ErrorListener::reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, size_t prediction, antlr4::atn::ATNConfigSet *configs) {

            // TMP
            REPORT(Comms::START_REPORT, Comms::CRITICAL_REPORT, "Context sensitivity reported from index ",
                startIndex ," to index " , stopIndex, Comms::END_REPORT);
            errorsDetected = true;
        }
    }
}
