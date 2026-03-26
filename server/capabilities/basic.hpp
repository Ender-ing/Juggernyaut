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

// Store
#include "../store/DocumentStore.hpp"

namespace Capabilities {
    extern lsp::MessageHandler *handler;
    extern void configureProtocol(lsp::MessageHandler &messageHandler, Store::DocumentStore &store, int &exit_code) ;
}
