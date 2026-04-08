/**
 * @brief 
 * Front-End Session
**/

#include "session.hpp"

// initiation
#include "init/parser.hpp"

// Store
#include "../data/store/SourceStore.hpp"

#ifdef NDEBUG
    // External library
    #include <mimalloc.h>
#endif

namespace Session {
    Session getSessionDefaults() {
        ParserConfigs parserConfigs; // Empty
        Configs sessionConfigs = {
            Stage::IRGen,
            std::move(parserConfigs)
        };

        ParserHooks parserHooks;
        Hooks sessionHooks = {
            std::move(parserHooks)
        };

        return {
            std::move(sessionConfigs),
            std::move(sessionHooks),
            nullptr
        };
    }

    // Main pipeline trigger funciton
    void initiate(const Session &session) {
        const Configs &configs = session.configs;

        // Check for musts
        if (session.store == nullptr) {
            return;
        }

        // [STAGE] Lexer & Parser
        Init::parser(session);
        if (configs.terminateAfter == Stage::Lexer || configs.terminateAfter == Stage::Parser) {
            return;
        }

        // [STAGE] Transformer (acts like a preprocessor)
        if (configs.terminateAfter == Stage::Transformer) {
            return;
        }

        // [STAGE] Semantic Analysis
        // ...
        if (configs.terminateAfter == Stage::Semantic) {
            return;
        }

        // [STAGE] LLVM IR Code Generation
        // ...
    }

    void rejuvenate(const Session &session) {
        // Delete unused documents
        Data::Store::SourceStore *store = session.store;
        store->cleanup();

        Init::rejuvenateParser();

        // Free unused memory
        #ifdef NDEBUG
            mi_collect(true);
        #endif
    }
}
