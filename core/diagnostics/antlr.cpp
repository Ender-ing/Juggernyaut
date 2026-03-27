/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#include "antlr.hpp"

// Common
#include "../common/utility.hpp"

namespace Diagnostics {
    int getAntlrErrorCode(const std::string &msg, std::exception_ptr e, bool &isMsgNum) {

        if (e) {
            try {
                std::rethrow_exception(e);
            } catch (const antlr4::LexerNoViableAltException &ex) {
                // Lexer: Unrecognized character
                ex;
                return 3;
            } catch (const antlr4::InputMismatchException &ex) {
                // Parser: Input mismatch
                ex;
                return 4;
            } catch (const antlr4::NoViableAltException &ex) {
                // Parser: No viable alternative
                ex;
                return 5;
            } catch (const antlr4::FailedPredicateException &ex) {
                // Semantics: Failed predicate
                ex;
                return 6; // MIGHT NEED TO REMOVE THIS LATER ON!
            } catch (const antlr4::RecognitionException &ex) {
                // Parser: Generic/Custom fallback
                ex;
                return 1;
            }
        } else {
            if (msg.find("missing") != std::string::npos) {
                // Parser: Missing token
                return 7;
            } else if (msg.find("extraneous") != std::string::npos) {
                // Parser: Extraneous token
                return 8;
            } else {
                if (Common::Utility::isNumber(msg)) {
                    isMsgNum = true;
                    return std::stoi(msg);
                } else {
                    // Parser: General auto-recovery
                    return 2;
                }
            }
        }
    }
    Diagnostic antlrToDiagnostic(antlr4::Recognizer *recognizer,
        antlr4::Token *offendingSymbol, size_t line, size_t charPositionInLine,
        const std::string &msg, std::exception_ptr e) {

        Diagnostic error;

        // Get the starting position
        error.range.start.line = (uint32_t) (line - 1);
        error.range.start.character = (uint32_t) charPositionInLine;

        // Determine the end position
        std::string tokenText;
        if (offendingSymbol != nullptr) {
            tokenText = offendingSymbol->getText();

            size_t newlineCount = std::count(tokenText.begin(), tokenText.end(), '\n');
            size_t endLine = error.range.start.line;
            size_t endCol = (uint32_t) charPositionInLine;

            // Handle newline offsets
            if (newlineCount > 0) {
                endLine += newlineCount;
                // After a newline, the column is just the length of the remaining string
                size_t lastNewlinePos = tokenText.find_last_of('\n');
                endCol = tokenText.length() - lastNewlinePos - 1;
            } else {
                endCol += tokenText.length();
            }

            // NOTE: Tokens shouldn't contain too much data.
            //     As such, you may ignore the data loss warnings!
            error.range.end.line = std::move(static_cast<uint32_t>(endLine));
            error.range.end.character = std::move(static_cast<uint32_t>(endCol));
        } else {
            // tokenText = "<N/A>";
            error.range.end.line = error.range.start.line;
            error.range.end.character = error.range.start.character + 1;
        }

        error.severity = Severity::Error;

        bool shouldFetchMessage = false;
        error.code = getAntlrErrorCode(msg, e, shouldFetchMessage);
        if (shouldFetchMessage) {
        } else {
            error.message = msg;
        }

        return error;
    }
}
