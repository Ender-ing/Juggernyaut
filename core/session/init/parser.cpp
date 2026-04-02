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
            Configs &configs = session.configs;
            Hooks &hooks = session.hooks;

            // Generate configs
            Parser::Configs parserConfigs;
            parserConfigs.terminateAfterLexer = (configs.terminateAfter == Stage::Lexer);

            Parser::sessionWorkflow(parserConfigs, hooks.parser);
        }
    }
}
