/**
 * @brief
 * Manage code diagnostics
 * (triggered by document content changes)
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// Front-End Driver
#include "./../../../core/driver/JugDriver.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

namespace Capabilities {
    namespace Semantics {
        extern void setupGlobalDiagnostics(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver) ;
    }
}
