/**
 * @brief
 * Jug language CST-to-AST lowering rules
**/

#pragma once

#include "../lowering/LoweringTable.hpp"

namespace Parser::Rules {
    void registerScopes(Lowering::LoweringTable &table) ;
    // void registerStatements(Lowering::LoweringTable &table);
    // void registerDeclarations(Lowering::LoweringTable &table);
    // void registerExpressions(Lowering::LoweringTable &table);

    [[nodiscard]] const Lowering::LoweringTable& getJugLoweringTable(ts::Language lang) ;
}
