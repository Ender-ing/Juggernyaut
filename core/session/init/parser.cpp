/**
 * @brief 
 * parser initiation
**/

#include "parser.hpp"

// Parser
#include "../../parser/parser.hpp"

namespace Session {
    namespace Init {
        void parser(const Session &session) {
            const Configs &configs = session.configs;
            const Hooks &hooks = session.hooks;
            Data::Store::SourceStore *store = session.store;

            // Generate configs
            Parser::Configs parserConfigs;
            parserConfigs.terminateAfterLexer = (configs.terminateAfter == Stage::Lexer);

            Parser::sessionWorkflow(parserConfigs, hooks.parser, store);
        }
    }
}
