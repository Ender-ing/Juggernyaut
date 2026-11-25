/**
 * @brief
 * Declarative CST-to-AST symbol & field role mapping table
**/

#include "ASTMapper.hpp"

// C++ headers
#include <utility>

namespace Parser::Visitors {
    ASTMapper::ASTMapper(ts::Language language)
        : lang(language),
          symbolRules(language.impl != nullptr ? language.getNumSymbols() : 0u),
          fieldRoles(language.impl != nullptr ? (language.getNumFields() + 1u) : 0u, ASTRole::None) {}

    // Registration API
    bool ASTMapper::bindNode(std::string_view cstRuleName, NodeRule rule, bool isNamed) {
        if (this->lang.impl == nullptr) {
            return false;
        }

        const std::optional<ts::Symbol> sym = this->lang.getSymbolForName(cstRuleName, isNamed);
        if (!sym.has_value()) {
            return false;
        }

        const size_t idx = std::to_underlying(*sym);
        if (idx >= this->symbolRules.size()) {
            return false;
        }

        rule.handled = true; // Update the rule's status
        this->symbolRules[idx] = rule;
        return true;
    }
    bool ASTMapper::bindField(std::string_view cstFieldName, ASTRole role) {
        if (this->lang.impl == nullptr) {
            return false;
        }

        const std::optional<ts::FieldId> fid = this->lang.getFieldId(cstFieldName);
        if (!fid.has_value()) {
            return false;
        }

        const size_t idx = std::to_underlying(*fid);
        if (idx >= this->fieldRoles.size()) {
            return false;
        }

        this->fieldRoles[idx] = role;
        return true;
    }

    // table lookups
    const NodeRule& ASTMapper::lookupRule(ts::Symbol symbol) const noexcept {
        const size_t idx = std::to_underlying(symbol);
        return idx < this->symbolRules.size() ? this->symbolRules[idx] : unhandledRule();
    }
    ASTRole ASTMapper::lookupFieldRole(ts::FieldId fieldId) const noexcept {
        const size_t idx = std::to_underlying(fieldId);
        return idx < this->fieldRoles.size() ? this->fieldRoles[idx] : ASTRole::None;
    }
    ASTRole ASTMapper::lookupFieldRole(std::string_view fieldName) const noexcept {
        if (this->lang.impl == nullptr) {
            return ASTRole::None;
        }

        const std::optional<ts::FieldId> fid = this->lang.getFieldId(fieldName);
        return fid.has_value() ? this->lookupFieldRole(*fid) : ASTRole::None;
    }
    ASTRole ASTMapper::resolveChildRole(ts::Node childNode) const noexcept {
        if (childNode.isNull() || !childNode.isNamed()) {
            return ASTRole::None;
        }

        const std::optional<ts::Node> cstParent = childNode.getParent();
        if (!cstParent.has_value()) {
            return ASTRole::None;
        }

        const ts::NodeID targetId = childNode.getID();

        // Look for childNode's role
        ts::TraversalCursor cursor;
        cursor.reset(*cstParent);
        if (cursor.gotoFirstChild()) {
            do {
                if (cursor.currentNode().getID() == targetId) {
                    return this->lookupFieldRole(cursor.currentFieldId());
                }
            } while (cursor.gotoNextSibling());
        }

        return ASTRole::None;
    }

    const NodeRule& ASTMapper::unhandledRule() noexcept {
        static constexpr NodeRule kUnhandled{};
        return kUnhandled;
    }
}
