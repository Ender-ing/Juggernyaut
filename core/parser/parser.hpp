/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "../common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Data
#include "../data/store/SourceStore.hpp"

// Parser
#include "Hooks.hpp"
#include "Configs.hpp"

namespace Parser {
    extern JUG_PARSER_API void sessionWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store) ;
}
