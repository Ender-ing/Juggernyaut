/**
 * @brief
 * Handle basic capabilities
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "../lspFramework.hpp"

// Session
#include "../../core/session/session.hpp"

namespace Capabilities {
    extern void configureProtocol(lsp::ServerEndpoint &endpoint, Session::Session &session, int &exit_code) ;
}
