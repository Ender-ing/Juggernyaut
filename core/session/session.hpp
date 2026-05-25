/**
 * @brief 
 * Front-End Session
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_SESSION_API

// Store
#include "../data/store/SourceStore.hpp"

// Parser
#include "../parser/Hooks.hpp"
#include "../parser/Configs.hpp"

// Data
#include "../data/manager/types.hpp"

namespace Session {
    // Session data
    enum Stage {
        Lexer=1,
        Parser=2,
        Transformer=3,
        Semantic=4,
        IaRC=5
    };

    // Core libs
    using ParserConfigs = Parser::SharedConfigs;
    using ParserHooks = Parser::Hooks; // Note that Lexer hooks are parser hooks!

    // Configs
    struct Configs {
        Stage terminateAfter = Stage::IaRC;
        ParserConfigs parser;
    };

    // Hooks are the way the session is supposed to interact with all other libraries!
    struct Hooks {
        ParserHooks parser;
    };

    // States are used to keep track of Session breaking changes that may require a forced-visit to certain session workflows!
    // WARNING: Only changes that may change the FINAL OUTPUT of the session may be considered breaking changes! 
    struct States {
        uint32_t parser = 0;
    };

    // Manager tools are used to communicate with the sessions manager
    // in order to facilitate inter-session communication and resource sharing.
    struct ManagerTools {
        // Data::Manager::ResolveLibraryPath resolveLibrary = nullptr;
    };

    struct Session {
        Configs configs;
        Hooks hooks;
        States states; // (Tracking for recurring meaningful session visits)
        Data::Store::SourceStore *store = nullptr;
        bool isRunning = false; // Tracking
        ManagerTools manager;
    };
    // [Session]
    // Musts: <Session>.store
    extern JUG_SESSION_API Session getSessionDefaults() ;
    // Main pipeline trigger funciton
    extern JUG_SESSION_API void initiate(Session &session) ;
    // Tidy up the session
    extern JUG_SESSION_API void rejuvenate(const Session &session) ;
}
