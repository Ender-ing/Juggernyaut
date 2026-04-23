/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#include "antlr.hpp"

// Common
#include "common/utility.hpp"

#include "DiagCodes.hpp"

namespace Diagnostics {
    int getAntlrErrorCode(const std::string &msg, std::exception_ptr e, bool &isMsgNum) {

        if (e) {
            try {
                std::rethrow_exception(e);
            } catch (const antlr4::LexerNoViableAltException &ex) {
                // Lexer: Unrecognized character
                (void)ex;
                return 100001;
            } catch (const antlr4::InputMismatchException &ex) {
                // Parser: Input mismatch
                (void)ex;
                return 200003;
            } catch (const antlr4::NoViableAltException &ex) {
                // Parser: No viable alternative
                (void)ex;
                return 200004;
            } catch (const antlr4::FailedPredicateException &ex) {
                // Semantics: Failed predicate
                (void)ex;
                return 200008; // MIGHT NEED TO REMOVE THIS LATER ON!
            } catch (const antlr4::RecognitionException &ex) {
                // Parser: Generic/Custom fallback
                (void)ex;
                return 200001;
            }
        } else {
            if (msg.find("missing") != std::string::npos) {
                // Parser: Missing token
                return 200005;
            } else if (msg.find("extraneous") != std::string::npos) {
                // Parser: Extraneous token
                return 200006;
            } else {
                if (Common::Utility::isNumber(msg)) {
                    isMsgNum = true;
                    return std::stoi(msg);
                } else {
                    // Parser: General auto-recovery
                    return 200007;
                }
            }
        }
    }

    Diagnostic antlrSyntaxErrorToDiagnostic(antlr4::Recognizer *recognizer,
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

            // The distance is expected to be positive
            size_t newlineCount = static_cast<size_t>(std::count(tokenText.begin(), tokenText.end(), '\n'));
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
            std::string customMsg;
            // TMP
            if (error.code == 100002) {
                customMsg = CODE_100002;
            } else if (error.code == 100003) {
                customMsg = CODE_100003;
            } else {
                customMsg = "Well, heck if I know... :/";
            }
            error.message = customMsg;
        } else {
            error.message = msg;
        }

        return error;
    }

    Diagnostic antlrAmbiguityToDiagnostic(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
        size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs) {

        Diagnostic warning;

        antlr4::TokenStream *tokens = recognizer->getTokenStream();
        std::string ambiguousText = tokens->getText(antlr4::misc::Interval(startIndex, stopIndex));

        // First token
        antlr4::Token *startToken = tokens->get(startIndex);
        size_t startLine = startToken->getLine();
        size_t startPos = startToken->getCharPositionInLine();

        // Last token
        antlr4::Token *stopToken = tokens->get(stopIndex);
        size_t endLine = stopToken->getLine();
        size_t endPos = stopToken->getCharPositionInLine() + stopToken->getText().length();

        // Update position data
        warning.range.start.line = (uint32_t) (startLine - 1);
        warning.range.start.character = (uint32_t) startPos;
        warning.range.end.line = (uint32_t) (endLine - 1);
        warning.range.end.character = (uint32_t) endPos;
        
        warning.severity = Severity::Warning;
        warning.code = 200002;
        warning.message = CODE_200002;

        return warning;
    }
}
