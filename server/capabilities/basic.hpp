/**
 * @brief
 * Handle basic capabilities
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "../lspFramework.hpp"

// Front-End Driver
#include "../../core/driver/JugDriver.hpp"

namespace Capabilities {
    extern void configureProtocol(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver, int &exit_code) ;
}
