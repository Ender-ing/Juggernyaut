/**
 * @brief 
 * Parser Abstract Syntax Tree
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_DATA_API

#include "ASTNode.hpp"

namespace Data::Parser {
    struct AST {
        std::vector<ASTNode> nodes;

        [[nodiscard]] bool empty() const noexcept ;

        [[nodiscard]] const ASTNode& root() const noexcept ;

        [[nodiscard]] std::span<const ASTNode> children(const ASTNode &node) const noexcept ;
        [[nodiscard]] const ASTNode* childByRole(const ASTNode &parent, ASTRole role) const noexcept ;
    };
}
