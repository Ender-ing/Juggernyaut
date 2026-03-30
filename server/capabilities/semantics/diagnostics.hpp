/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

// Parser
#include "../../../core/parser/parser.hpp"
#include "../../../core/parser/listeners/DiagnosticListener.hpp"

// Store
#include "../../store/DocumentStore.hpp"

namespace Capabilities {
    namespace Semantics {
        // TMP
        // Listen for syntax-related errors
        class SyntaxListener : public Parser::Listeners::DiagnosticListener {
            private:
                std::vector<lsp::Diagnostic> &diags;
            public:
                // Constructors
                SyntaxListener(std::vector<lsp::Diagnostic> &diagsVector) : diags(diagsVector) { }

                // ANTLR4 functions
                void onSyntaxError(Diagnostics::Diagnostic) override ;
                void onAmbiguity(Diagnostics::Diagnostic) override ;
        };

        extern void validateDocumentSyntax(lsp::MessageHandler &messageHandler, Store::Document &doc) ;
    }
}
