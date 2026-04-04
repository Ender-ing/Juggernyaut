/**
 * @brief
 * Lexer & parser error listeners
**/

#include "ASTGenListener.hpp"

namespace Parser {
    namespace Listeners {
        namespace Internal {
            Diagnostics::Diagnostic getGenTokenDiagnostic(antlr4::Token *token) {
                Diagnostics::Diagnostic diag;

                if (token == nullptr) {
                    return diag;
                }

                const size_t posInLine = token->getCharPositionInLine();
                const size_t tokenLength = (token->getText()).length();

                // Update position data
                diag.range.start.line = (uint32_t) (token->getLine() - 1);
                diag.range.start.character = (uint32_t) posInLine;
                diag.range.end.line = diag.range.end.line;
                diag.range.end.character = (uint32_t) (posInLine + tokenLength);
        
                return diag;
            }
            Diagnostics::Diagnostic getGenNodeDiagnostic(antlr4::tree::TerminalNode *node) {
                Diagnostics::Diagnostic diag;

                if (node == nullptr) {
                    return diag;
                }

                // Get token
                antlr4::Token* token = node->getSymbol();
                diag = getGenTokenDiagnostic(token);

                return diag;
            }
            Diagnostics::Diagnostic getGenRuleDiagnostic(antlr4::ParserRuleContext *context) {
                Diagnostics::Diagnostic diag;

                if (context == nullptr) {
                    return diag;
                }

                antlr4::Token* startToken = context->getStart();
                antlr4::Token* stopToken = context->getStop();

                // Empty stream match
                if (startToken == nullptr || stopToken == nullptr) {
                    return diag;
                }

                // Update start position data
                diag.range.start.line = (uint32_t) (startToken->getLine() - 1);
                diag.range.start.character = (uint32_t) startToken->getCharPositionInLine();

                // Update end position data
                size_t stopTokenLength = stopToken->getText().length();
                diag.range.end.line = (uint32_t) (stopToken->getLine() - 1);
                diag.range.end.character = (uint32_t) (stopToken->getCharPositionInLine() + stopTokenLength);

                return diag;
            }
        }

        void ASTGenListener::pushDiagnostic(Diagnostics::Diagnostic &diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = this->src->getId();

            source->addParserDiagnostic(std::move(diag));
        }
    }
}
