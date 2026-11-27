/**
 * @brief
 * Jug statement lowering rules umbrella
**/

#pragma once

#include "../../lowering/LoweringTable.hpp"

namespace Parser::Rules {
    JUG_PARSER_API void registerStatements(Lowering::LoweringTable &table) ;
}
