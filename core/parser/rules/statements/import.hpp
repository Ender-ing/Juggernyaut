/**
 * @brief
 * Jug `import` statement lowering rules & dependency extraction
**/

#pragma once

#include "../../lowering/LoweringTable.hpp"

namespace Parser::Rules::Statements {
    JUG_PARSER_API void registerImport(Lowering::LoweringTable &table) ;
}
