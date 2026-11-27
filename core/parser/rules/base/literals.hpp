/**
 * @brief
 * Jug literal decoding, validation, and lowering rules
**/

#pragma once

#include "../../lowering/LoweringTable.hpp"

namespace Parser::Rules::Base {
    // Strips outer quotes and decodes Jug escape sequences (`\n`, `\t`, `\\`, line continuations, etc.)
    [[nodiscard]] JUG_PARSER_API std::string unescapeStringLiteral(std::string_view raw) ;

    JUG_PARSER_API void registerLiterals(Lowering::LoweringTable &table) ;
}
