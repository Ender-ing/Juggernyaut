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

// Session
#include "../../../core/session/session.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

namespace Capabilities {
    namespace Semantics {
        extern void setupParserDiagnostics(lsp::MessageHandler &messageHandler, Session::Session &session) ;
    }
}
