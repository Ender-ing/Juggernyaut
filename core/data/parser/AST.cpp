/**
 * @brief 
 * Parser Abstract Syntax Tree
**/

#include "AST.hpp"

// C++ headers
#include <algorithm>

namespace Data::Parser {
    [[nodiscard]] bool AST::empty() const noexcept {
        return this->nodes.empty();
    }
    [[nodiscard]] const ASTNode& AST::root() const noexcept {
        return this->nodes.front();
    }

    [[nodiscard]] std::span<const ASTNode> AST::children(const ASTNode &target) const noexcept {
        if (target.childCount == 0u ||
            (static_cast<size_t>(target.firstChild) + target.childCount) > this->nodes.size()) {
            return {};
        }
        return std::span<const ASTNode>(this->nodes.data() + target.firstChild, target.childCount);
    }
    [[nodiscard]] const ASTNode* AST::childByRole(const ASTNode &parent, ASTRole role) const noexcept {
        for (const ASTNode &child : this->children(parent)) {
            if (child.role == role) {
                return &child;
            }
        }
        return nullptr;
    }

    [[nodiscard]] LexemeId AST::pushLexeme(std::string_view text) {
        const auto id = static_cast<LexemeId>(this->lexemes.size());
        this->lexemes.emplace_back(text);
        return id;
    }
    [[nodiscard]] std::string_view AST::lexemeText(LexemeId id) const noexcept {
        return id < this->lexemes.size() ? std::string_view(this->lexemes[id]) : std::string_view{};
    }
    [[nodiscard]] size_t AST::internedLexemeCount() const noexcept {
        return this->lexemes.size();
    }
    [[nodiscard]] std::string_view AST::textOf(const ASTNode &target, const Host::File &sourceFile) const noexcept {
        const std::string_view src = sourceFile.content.view();
        if (target.span.start >= src.size() || target.span.end <= target.span.start) {
            return {};
        }
        const size_t len = std::min<size_t>(target.span.length(), src.size() - target.span.start);
        return src.substr(target.span.start, len);
    }
    [[nodiscard]] Diagnostics::Range AST::rangeOf(const ASTNode &target, const Host::File &sourceFile) const noexcept {
        const Host::TextPoint startPt = sourceFile.content.pointAtByte(target.span.start);
        const Host::TextPoint endPt = sourceFile.content.pointAtByte(target.span.end);
        return Diagnostics::Range{
            .start = Diagnostics::Position{.line = startPt.line, .character = startPt.column},
            .end = Diagnostics::Position{.line = endPt.line, .character = endPt.column},
        };
    }
}
