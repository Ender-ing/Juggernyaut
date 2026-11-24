/**
 * @brief
 * AST rules & scopes mapper
**/

#pragma once
#include "../visitors/ASTMapper.hpp"

namespace Parser::Rules {
    void registerScopes(Visitors::ASTMapper &mapper) ;

    // Get a static AST mapper
    [[nodiscard]] const Visitors::ASTMapper& getJugASTMapper(ts::Language lang) ;
}
