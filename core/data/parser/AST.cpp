/**
 * @brief 
 * Parser Abstract Syntax Tree
**/

#include "AST.hpp"

namespace Data::Parser {
    [[nodiscard]] bool AST::empty() const noexcept {
        return this->nodes.empty();
    }

    [[nodiscard]] const ASTNode& AST::root() const noexcept {
        return this->nodes.front();
    }

    [[nodiscard]] std::span<const ASTNode> AST::children(const ASTNode &node) const noexcept {
        if (node.childCount == 0 || (node.firstChild + node.childCount) > this->nodes.size()) {
            return {};
        }
        return std::span<const ASTNode>(this->nodes.data() + node.firstChild, node.childCount);
    }
    [[nodiscard]] const ASTNode* AST::childByRole(const ASTNode &parent, ASTRole role) const noexcept {
        for (const ASTNode &child : this->children(parent)) {
            if (child.role == role) {
                return &child;
            }
        }
        return nullptr;
    }
}
