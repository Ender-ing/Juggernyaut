/**
 * @brief
 * Immutable O(1) CST-to-AST symbol & scoped field-role lowering table
**/

#include "LoweringTable.hpp"

namespace Parser::Lowering {
    LoweringTable::LoweringTable(ts::Language language)
        : lang(language),
          fieldStride(language.impl != nullptr ? (language.getNumFields() + 1u) : 0u),
          roleRows(this->fieldStride > 0u ? 1u : 0u),
          symbolRules(language.impl != nullptr ? language.getNumSymbols() : 0u),
          roleMatrix(this->fieldStride, ASTRole::None) {}
}
