/**
 * @brief
 * Jug scope & block lowering rules
**/

#include "scopes.hpp"

namespace Parser::Rules {
    void registerScopes(Lowering::LoweringTable &table) {
        table.bindNode("prog", Data::Parser::ASTKind::Program);
        table.bindNode("generic_block", Data::Parser::ASTKind::Block);
    }
}
