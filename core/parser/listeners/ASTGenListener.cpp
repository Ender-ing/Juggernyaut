/**
 * @brief
 * Lexer & parser error listeners
**/

#include "ASTGenListener.hpp"

namespace Parser {
    namespace Listeners {
        void ASTGenListener::pushDiagnostic(Diagnostics::Diagnostic &diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = this->src->getId();

            source->addParserDiagnostic(std::move(diag));
        }
    }
}
