/**
 * @brief
 * Manage configs onloading
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

// Front-End Driver
#include "../../../core/driver/JugDriver.hpp"
#include "../../driver/DriverDebouncer.hpp"

namespace Capabilities {
    namespace Docs {
        extern void registerConfigsWatcher(lsp::ServerEndpoint &endpoint, Driver::JugDriver::JugDriver &driver,
            std::unique_ptr<Driver::DriverDebouncer> &debouncer, const std::string &configUri) ;
    }
}
