/**
 * @brief 
 * Front-End Session
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_SESSION_API

// Store
#include "../data/store/SourceStore.hpp"

// Parser
#include "../parser/Hooks.hpp"

namespace Session {
    // Session data
    enum Stage {
        Lexer=1,
        Parser=2,
        Transformer=3,
        Semantic=4,
        IRGen=5 // Strictly needed for the compiler - easier to handle here!
    };
    struct Configs {
        Stage terminateAfter = Stage::IRGen;
    };

    // Core libs
    using ParserHooks = Parser::Hooks; // Note that Lexer hooks are parser hooks!

    // Hooks are the way the session is supposed to interact with all other libraries!
    struct Hooks {
        ParserHooks &parser;
    };

    struct Session {
        Configs &configs;
        Hooks &hooks;
        Data::Store::SourceStore &store;
    };
   // Main pipeline trigger funciton
    extern JUG_SESSION_API void initiate(const Session &session) ;
}
