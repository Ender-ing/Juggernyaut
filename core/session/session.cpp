/**
 * @brief 
 * Front-End Session
**/

#include "session.hpp"

// initiation
#include "init/parser.hpp"

namespace Session {
    // Main pipeline trigger funciton
    void initiate(const Configs &configs, const Hooks &hooks) {
        // [STAGE] Lexer & Parser
        Init::parser(configs, hooks);
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
}
