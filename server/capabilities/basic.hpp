/**
 * @brief
 * Handle basic capabilities
**/

#pragma once

// Common headers
#include "../../core/common/headers.hpp"

// lsp-framework
#include "../lspFramework.hpp"

#include "print.hpp"

// Session
#include "../../core/session/session.hpp"

namespace Capabilities {
    extern void configureProtocol(lsp::MessageHandler &messageHandler, Session::Session &session, int &exit_code) ;
}
