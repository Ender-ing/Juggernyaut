/**
 * @brief
 * Handle diagnostic for shared contexts
**/

#include "Diagnostic.hpp"

// Common
#include "../common/utility.hpp"

namespace Diagnostics {
    int getAntlrErrorCode(const std::string &msg, std::exception_ptr e) {

        if (e) {
            try {
                std::rethrow_exception(e);
            } catch (const antlr4::LexerNoViableAltException &ex) {
                // Lexer: Unrecognized character
                ex;
                return 1;
            } catch (const antlr4::InputMismatchException &ex) {
                // Parser: Input mismatch
                ex;
                return 2;
            } catch (const antlr4::NoViableAltException &ex) {
                // Parser: No viable alternative
                ex;
                return 3;
            } catch (const antlr4::FailedPredicateException &ex) {
                // Semantics: Failed predicate
                ex;
                return 1; // MIGHT NEED TO REMOVE THIS LATER ON!
            } catch (const antlr4::RecognitionException &ex) {
                // Parser: Generic/Custom fallback
                ex;
                return 1;
            }
        } else {
            if (msg.find("missing") != std::string::npos) {
                // Parser: Missing token
                return 4;
            } else if (msg.find("extraneous") != std::string::npos) {
                // Parser: Extraneous token
                return 5;
            } else {
                // Parser: General auto-recovery
                return 6;
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
            // TMP
            tokenText = offendingSymbol->getText();
            error.range.end.character = (uint32_t) (charPositionInLine + tokenText.length());
        } else {
            // TMP
            // tokenText = "<N/A>";
            error.range.end.character = error.range.start.character + 1;
        }
        error.range.end.line = error.range.start.line; // No token is allowed to contain newlines!

        error.severity = Severity::Error;

        error.message = msg;
        error.code = getAntlrErrorCode(msg, e);

        return error;
    }
}
