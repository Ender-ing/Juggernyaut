/**
 * @brief
 * Jug identifier terminal lowering rules
**/

#include "identifiers.hpp"

namespace Parser::Rules::Base {
    void registerIdentifiers(Lowering::LoweringTable &table) {
        table.bindLeaf("identifier", Data::Parser::ASTKind::Identifier);
    }
}
