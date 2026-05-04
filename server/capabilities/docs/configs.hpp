/**
 * @brief
 * Manage configs onloading
**/

#pragma once

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "../../lspFramework.hpp"

// Session
#include "../../../core/session/session.hpp"

// Configs
#include "../../../configs/configs.hpp"

#include "../../session/SessionDebouncer.hpp"

namespace Capabilities {
    namespace Docs {
        extern Configs::BreakingChanges updateSessionConfigs(lsp::MessageHandler &messageHandler,
            Session::Session &session, const std::string &configUri) ;
        extern void registerConfigsWatcher(lsp::MessageHandler &messageHandler, Session::Session &session,
            std::unique_ptr<Session::SessionDebouncer> &debouncer, const std::string &configUri) ;
    }
}
