/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Parser
#include "Hooks.hpp"
#include "Configs.hpp"

namespace Parser {
    //extern JUG_PARSER_API void sessionWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store, uint32_t parserState) ;
    //extern JUG_PARSER_API void cleanup() ;
    class JUG_PARSER_API Parser {
        private:
            // ...
        public:
            // NOTE: Session functions may operate on the assumption that 'hostVFS' and 'unitEnv' actually link to proper memory addresses!
            Parser() {}
            ~Parser() = default;

            // Non-copyable, movable
            Parser(const Parser&) = delete;
            Parser& operator=(const Parser&) = delete;
            Parser(Parser&&) noexcept = default;
            Parser& operator=(Parser&&) noexcept = default;
    };
}
