/**
 * @brief
 * Immutable O(1) CST-to-AST symbol & scoped field-role lowering table
**/

#include "LoweringTable.hpp"

// C++ headers
#include <utility>

namespace Parser::Lowering {
    LoweringTable::LoweringTable(ts::Language language)
        : lang(language),
          fieldStride(language.impl != nullptr ? (language.getNumFields() + 1u) : 0u),
          roleRows(this->fieldStride > 0u ? 1u : 0u),
          symbolRules(language.impl != nullptr ? language.getNumSymbols() : 0u),
          roleMatrix(this->fieldStride, ASTRole::None) {}

    // Symbol Registration API
    bool LoweringTable::bindSymbol(std::string_view symbolName, SymbolRule rule, bool isNamed) {
        if (this->lang.impl == nullptr) {
            return false;
        }

        const std::optional<ts::Symbol> sym = this->lang.getSymbolForName(symbolName, isNamed);
        if (!sym.has_value()) {
            return false;
        }

        const size_t idx = std::to_underlying(*sym);
        if (idx >= this->symbolRules.size()) {
            return false;
        }

        this->symbolRules[idx] = rule;
        return true;
    }
    bool LoweringTable::bindNode(std::string_view ruleName, ASTKind kind, bool resolveChildRoles,
        SymbolRule::Hook onEnter, SymbolRule::Hook onLeave) {
        return this->bindSymbol(
            ruleName,
            SymbolRule{
                .op = LoweringOp::EmitNode,
                .astKind = kind,
                .resolveChildRoles = resolveChildRoles,
                .onEnter = onEnter,
                .onLeave = onLeave,
            },
            true
        );
    }
    bool LoweringTable::bindNode(std::string_view ruleName, SymbolRule rule) {
        if (rule.op == LoweringOp::Unhandled) {
            rule.op = (rule.astKind != ASTKind::None) ? LoweringOp::EmitNode : LoweringOp::PassThrough;
        }
        return this->bindSymbol(ruleName, rule, true);
    }
    bool LoweringTable::bindPassThrough(std::string_view ruleName, bool forwardField,
        SymbolRule::Hook onEnter, SymbolRule::Hook onLeave) {
        return this->bindSymbol(
            ruleName,
            SymbolRule{
                .op = LoweringOp::PassThrough,
                .forwardField = forwardField,
                .onEnter = onEnter,
                .onLeave = onLeave,
            },
            true
        );
    }
    bool LoweringTable::bindParentModifier(std::string_view tokenName, uint8_t subKind, uint8_t flags,
        ASTKind targetParent, ASTKind overrideKind, bool isNamed) {
        return this->bindSymbol(
            tokenName,
            SymbolRule{
                .op = LoweringOp::MutateParent,
                .astKind = overrideKind,
                .targetParent = targetParent,
                .subKind = subKind,
                .flags = flags,
            },
            isNamed
        );
    }
    bool LoweringTable::bindIgnore(std::string_view symbolName, bool isNamed) {
        return this->bindSymbol(symbolName, SymbolRule{.op = LoweringOp::Ignore}, isNamed);
    }

    // Field Role Registration API
    bool LoweringTable::bindDefaultRole(std::string_view fieldName, ASTRole role) {
        return this->bindScopedRole(ASTKind::None, fieldName, role);
    }
    bool LoweringTable::bindScopedRole(ASTKind parentKind, std::string_view fieldName, ASTRole role) {
        if (this->lang.impl == nullptr || this->fieldStride == 0u) {
            return false;
        }

        const std::optional<ts::FieldId> fid = this->lang.getFieldId(fieldName);
        if (!fid.has_value()) {
            return false;
        }

        const size_t col = std::to_underlying(*fid);
        if (col == 0u || col >= this->fieldStride) {
            return false;
        }

        const size_t row = std::to_underlying(parentKind);
        if (row >= this->roleRows) {
            const uint32_t nextRows = static_cast<uint32_t>(row + 1u);
            this->roleMatrix.resize(static_cast<size_t>(nextRows) * this->fieldStride, ASTRole::None);
            this->roleRows = nextRows;
        }

        this->roleMatrix[row * this->fieldStride + col] = role;
        return true;
    }

    // Hot-Path O(1) Lookups
    const SymbolRule& LoweringTable::lookupSymbol(ts::Symbol symbol) const noexcept {
        const size_t idx = std::to_underlying(symbol);
        return idx < this->symbolRules.size() ? this->symbolRules[idx] : unhandledRule();
    }
    ASTRole LoweringTable::lookupRole(ASTKind parentKind, ts::FieldId fieldId) const noexcept {
        const size_t col = std::to_underlying(fieldId);
        if (col == 0u || col >= this->fieldStride) {
            return ASTRole::None;
        }

        const size_t row = std::to_underlying(parentKind);
        if (row > 0u && row < this->roleRows) {
            const ASTRole scoped = this->roleMatrix[row * this->fieldStride + col];
            if (scoped != ASTRole::None) {
                return scoped;
            }
        }
        return this->roleMatrix[col]; // Row 0: global default role
    }

    const SymbolRule& LoweringTable::unhandledRule() noexcept {
        static constexpr SymbolRule kUnhandled{};
        return kUnhandled;
    }
}
