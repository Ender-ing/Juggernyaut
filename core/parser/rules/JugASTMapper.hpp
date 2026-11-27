/**
 * @brief
 * Jug language CST-to-AST lowering rules
**/

#pragma once

#include "../lowering/LoweringTable.hpp"

// Rule categories
#include "scopes.hpp"
#include "statements/statements.hpp"

namespace Parser::Rules {
    JUG_PARSER_API void registerBase(Lowering::LoweringTable &table) ;
    // void registerDeclarations(Lowering::LoweringTable &table);
    // void registerExpressions(Lowering::LoweringTable &table);

    [[nodiscard]] JUG_PARSER_API const Lowering::LoweringTable& getJugLoweringTable(ts::Language lang) ;
}
