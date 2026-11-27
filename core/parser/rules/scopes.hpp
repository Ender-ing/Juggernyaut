/**
 * @brief
 * Jug scope & block lowering rules
**/

#pragma once

#include "../lowering/LoweringTable.hpp"

namespace Parser::Rules {
    JUG_PARSER_API void registerScopes(Lowering::LoweringTable &table) ;
}
