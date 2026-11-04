/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Diagnostics
#include "../diagnostics./Diagnostic.hpp"

// Parser
#include "Hooks.hpp"
#include "Configs.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Parser {
    using File = Data::Host::File;
    using Range = Diagnostics::Range;

    struct ImportRequest {
        bool isExternal = false; // 'true' only when importing libraries!
        std::string target = "";
        std::string alias = "";
        std::vector<std::string> symbols;
        Range range{};
    };

    //extern JUG_PARSER_API void sessionWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store, uint32_t parserState) ;
    //extern JUG_PARSER_API void cleanup() ;
    class JUG_PARSER_API Parser {
        private:
            Configs configs{};
            Hooks hooks{};
        public:
            // NOTE: Session functions may operate on the assumption that 'hostVFS' and 'unitEnv' actually link to proper memory addresses!
            Parser(Configs parserConfigs = {}, Hooks parserHooks = {})
                : configs(std::move(parserConfigs)), hooks(std::move(parserHooks)) {}
            ~Parser() = default;

            // Non-copyable, movable
            Parser(const Parser&) = delete;
            Parser& operator=(const Parser&) = delete;
            Parser(Parser&&) noexcept = default;
            Parser& operator=(Parser&&) noexcept = default;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
