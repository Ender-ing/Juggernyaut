/**
 * @brief
 * Stateful per-parse CSTConsumer that assembles flat ASTs using a LoweringTable
**/

#include "ASTBuilder.hpp"

namespace Parser::Lowering {
    ASTBuilder::ASTBuilder(const LoweringTable *loweringTable) noexcept
        : table(loweringTable) {}
}
