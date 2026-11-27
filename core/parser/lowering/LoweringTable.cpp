/**
 * @brief
 * Immutable O(1) CST-to-AST symbol & scoped field-role lowering table
**/

#include "LoweringTable.hpp"

// C++ headers
#include <utility>

namespace Parser::Lowering {
    namespace Helper {
        [[nodiscard]] constexpr uint8_t scopeSpecificityRank(ts::Symbol cstParent, ASTKind astParent) noexcept {
            const bool hasCST = (cstParent != LoweringTable::ANY_CST_SYMBOL);
            const bool hasAST = (astParent != ASTKind::None);
            if (hasCST && hasAST) {
                return 0u;
            }
            if (hasCST) {
                return 1u;
            }
            return 2u;
        }
    }

    void LoweringTable::resolveSymbols(std::string_view name, bool isNamed, std::vector<ts::Symbol> &out) const {
        out.clear();
        if (this->lang.impl == nullptr || name.empty()) {
            return;
        }

        const uint32_t total = ts_language_symbol_count(this->lang.impl);
        const TSSymbolType expectedType = isNamed ? TSSymbolTypeRegular : TSSymbolTypeAnonymous;
        for (uint32_t raw = 0; raw < total; ++raw) {
            const auto tsSym = static_cast<TSSymbol>(raw);
            if (ts_language_symbol_type(this->lang.impl, tsSym) != expectedType) {
                continue;
            }
            const char *symName = ts_language_symbol_name(this->lang.impl, tsSym);
            if (symName != nullptr && std::string_view(symName) == name) {
                out.push_back(static_cast<ts::Symbol>(static_cast<RawSymbol>(raw)));
            }
        }
    }
    bool LoweringTable::insertScopedSymbolRule(ts::Symbol sym, ts::Symbol cstParent,
        ASTKind astParent, const SymbolRule &rule, std::string_view debugSymName,
        std::string_view debugScopeName) {
        const size_t idx = std::to_underlying(sym);
        if (idx >= this->symbolScopeRanges.size()) {
            return false;
        }

        ScopedSlice &slice = this->symbolScopeRanges[idx];
        for (uint32_t i = 0; i < slice.count; ++i) {
            const ScopedSymbolRule &existing = this->scopedRules[slice.offset + i];
            if (existing.cstParent == cstParent && existing.astParent == astParent) {
                this->issues.push_back(RegistrationIssue{
                    .kind = RegistrationIssueKind::DuplicateSymbolRule,
                    .targetName = std::string(debugSymName),
                    .scopeName = std::string(debugScopeName),
                    .parentKind = astParent,
                });
                return false;
            }
        }
        const ScopedSymbolRule entry{
            .cstParent = cstParent,
            .astParent = astParent,
            .rule = rule,
        };

        if (slice.count == 0u) {
            slice.offset = static_cast<uint32_t>(this->scopedRules.size());
            slice.count = 1u;
            this->scopedRules.push_back(entry);
            return true;
        }

        const uint8_t newRank = Helper::scopeSpecificityRank(cstParent, astParent);
        uint32_t relPos = slice.count;
        for (uint32_t i = 0; i < slice.count; ++i) {
            const ScopedSymbolRule &curr = this->scopedRules[slice.offset + i];
            if (newRank < Helper::scopeSpecificityRank(curr.cstParent, curr.astParent)) {
                relPos = i;
                break;
            }
        }

        const uint32_t insertIdx = slice.offset + relPos;
        this->scopedRules.insert(
            this->scopedRules.begin() + static_cast<std::ptrdiff_t>(insertIdx),
            entry
        );
        ++slice.count;

        for (size_t k = 0; k < this->symbolScopeRanges.size(); ++k) {
            if (k == idx) {
                continue;
            }
            ScopedSlice &other = this->symbolScopeRanges[k];
            if (other.count > 0u && other.offset >= slice.offset) {
                ++other.offset;
            }
        }
        return true;
    }


    LoweringTable::LoweringTable(ts::Language language)
        : lang(language),
          fieldStride(language.impl != nullptr ? (language.getNumFields() + 1u) : 0u),
          roleRows(this->fieldStride > 0u ? uint16_t{1u} : uint16_t{0u}),
          symbolRules(language.impl != nullptr ? language.getNumSymbols() : 0u),
          symbolScopeRanges(this->symbolRules.size()),
          roleMatrix(this->fieldStride, ASTRole::None) {}

    // Symbol Registration API
    bool LoweringTable::bindSymbol(std::string_view symbolName, SymbolRule rule, bool isNamed) {
        std::vector<ts::Symbol> matches;
        this->resolveSymbols(symbolName, isNamed, matches);
        if (matches.empty()) {
            this->issues.push_back(RegistrationIssue{
                .kind = RegistrationIssueKind::MissingSymbol,
                .targetName = std::string(symbolName),
            });
            return false;
        }

        bool ok = true;
        for (const ts::Symbol sym : matches) {
            const size_t idx = std::to_underlying(sym);
            if (idx >= this->symbolRules.size()) {
                ok = false;
                continue;
            }
            if (this->symbolRules[idx].op != LoweringOp::Unhandled) {
                this->issues.push_back(RegistrationIssue{
                    .kind = RegistrationIssueKind::DuplicateSymbolRule,
                    .targetName = std::string(symbolName),
                });
                ok = false;
                continue;
            }
            this->symbolRules[idx] = rule;
        }
        return ok;
    }
    bool LoweringTable::bindScopedSymbol(std::string_view symbolName, SymbolRule rule, bool isNamed,
        std::string_view cstParentName, ASTKind astParent, bool cstParentIsNamed) {
        if (cstParentName.empty() && astParent == ASTKind::None) {
            return this->bindSymbol(symbolName, rule, isNamed);
        }

        std::vector<ts::Symbol> symMatches;
        this->resolveSymbols(symbolName, isNamed, symMatches);
        if (symMatches.empty()) {
            this->issues.push_back(RegistrationIssue{
                .kind = RegistrationIssueKind::MissingSymbol,
                .targetName = std::string(symbolName),
                .scopeName = std::string(cstParentName),
                .parentKind = astParent,
            });
            return false;
        }

        std::vector<ts::Symbol> parentMatches;
        if (cstParentName.empty()) {
            parentMatches.push_back(ANY_CST_SYMBOL);
        } else {
            this->resolveSymbols(cstParentName, cstParentIsNamed, parentMatches);
            if (parentMatches.empty()) {
                this->issues.push_back(RegistrationIssue{
                    .kind = RegistrationIssueKind::MissingParentSymbol,
                    .targetName = std::string(symbolName),
                    .scopeName = std::string(cstParentName),
                    .parentKind = astParent,
                });
                return false;
            }
        }

        bool ok = true;
        for (const ts::Symbol sym : symMatches) {
            for (const ts::Symbol cstParent : parentMatches) {
                if (!this->insertScopedSymbolRule(sym, cstParent, astParent, rule, symbolName, cstParentName)) {
                    ok = false;
                }
            }
        }
        return ok;
    }
    bool LoweringTable::bindSymbolInCSTParent(std::string_view cstParentName, std::string_view symbolName,
        SymbolRule rule, bool isNamed) {
        return this->bindScopedSymbol(symbolName, rule, isNamed, cstParentName, ASTKind::None, true);
    }
    bool LoweringTable::bindSymbolForParentKind(ASTKind astParent, std::string_view symbolName,
        SymbolRule rule, bool isNamed) {
        return this->bindScopedSymbol(symbolName, rule, isNamed, {}, astParent, true);
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
    bool LoweringTable::bindLeaf(std::string_view ruleName, ASTKind kind, bool internText,
        SymbolRule::Hook onLeave) {
        return this->bindSymbol(
            ruleName,
            SymbolRule{
                .op = LoweringOp::EmitNode,
                .astKind = kind,
                .internText = internText,
                .onLeave = onLeave,
            },
            true
        );
    }
    bool LoweringTable::bindPrefixNode(std::string_view ruleName, ASTKind kind, bool resolveChildRoles,
        SymbolRule::Hook onEnter, SymbolRule::Hook onLeave) {
        return this->bindSymbol(
            ruleName,
            SymbolRule{
                .op = LoweringOp::EmitNode,
                .astKind = kind,
                .resolveChildRoles = resolveChildRoles,
                .attachToNext = true,
                .onEnter = onEnter,
                .onLeave = onLeave,
            },
            true
        );
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

    bool LoweringTable::bindParentModifier(std::string_view tokenName, uint16_t subKind,
        uint16_t flags, ASTKind targetParent, ASTKind overrideKind, bool isNamed,
        bool clobberSubKind) {
        const SymbolRule rule{
            .op = LoweringOp::MutateParent,
            .astKind = overrideKind,
            .targetParent = targetParent,
            .subKind = subKind,
            .flags = flags,
            .clobberSubKind = clobberSubKind,
        };
        if (targetParent != ASTKind::None) {
            return this->bindSymbolForParentKind(targetParent, tokenName, rule, isNamed);
        }
        return this->bindSymbol(tokenName, rule, isNamed);
    }
    bool LoweringTable::bindCSTParentModifier(std::string_view cstParentName, std::string_view tokenName,
        uint16_t subKind, uint16_t flags, ASTKind targetParent, ASTKind overrideKind,
        bool isNamed, bool clobberSubKind) {
        return this->bindScopedSymbol(
            tokenName,
            SymbolRule{
                .op = LoweringOp::MutateParent,
                .astKind = overrideKind,
                .targetParent = targetParent,
                .subKind = subKind,
                .flags = flags,
                .clobberSubKind = clobberSubKind,
            },
            isNamed,
            cstParentName,
            targetParent,
            true
        );
    }
    bool LoweringTable::bindPrefixModifier(std::string_view tokenName, uint16_t subKind,
        uint16_t flags, std::string_view cstParentName, bool isNamed, bool clobberSubKind) {
        const SymbolRule rule{
            .op = LoweringOp::MutateNext,
            .subKind = subKind,
            .flags = flags,
            .clobberSubKind = clobberSubKind,
        };
        if (!cstParentName.empty()) {
            return this->bindSymbolInCSTParent(cstParentName, tokenName, rule, isNamed);
        }
        return this->bindSymbol(tokenName, rule, isNamed);
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
            this->issues.push_back(RegistrationIssue{
                .kind = RegistrationIssueKind::MissingField,
                .targetName = std::string(fieldName),
                .parentKind = parentKind,
            });
            return false;
        }

        const size_t col = std::to_underlying(*fid);
        if (col == 0u || col >= this->fieldStride) {
            return false;
        }

        uint16_t compactRow = 0u;
        const size_t kindIdx = std::to_underlying(parentKind);
        if (kindIdx > 0u) {
            if (kindIdx >= this->kindToRoleRow.size()) {
                this->kindToRoleRow.resize(kindIdx + 1u, 0u);
            }
            compactRow = this->kindToRoleRow[kindIdx];
            if (compactRow == 0u) {
                compactRow = this->roleRows++;
                this->kindToRoleRow[kindIdx] = compactRow;
                this->roleMatrix.resize(static_cast<size_t>(this->roleRows) * this->fieldStride, ASTRole::None);
            }
        }

        ASTRole &cell = this->roleMatrix[static_cast<size_t>(compactRow) * this->fieldStride + col];
        if (cell != ASTRole::None) {
            this->issues.push_back(RegistrationIssue{
                .kind = RegistrationIssueKind::DuplicateFieldRole,
                .targetName = std::string(fieldName),
                .parentKind = parentKind,
            });
            return false;
        }

        cell = role;
        return true;
    }

    // Registration Validation
    bool LoweringTable::hasRegistrationIssues() const noexcept {
        return !this->issues.empty();
    }
    std::span<const RegistrationIssue> LoweringTable::registrationIssues() const noexcept {
        return this->issues;
    }

    // Hot-Path O(1) Lookups
    const SymbolRule& LoweringTable::lookupSymbol(ts::Symbol symbol) const noexcept {
        const size_t idx = std::to_underlying(symbol);
        return idx < this->symbolRules.size() ? this->symbolRules[idx] : unhandledRule();
    }
    const SymbolRule& LoweringTable::lookupSymbol(ts::Symbol symbol, ts::Symbol cstParentSymbol,
        ASTKind astParentKind) const noexcept {
        const size_t idx = std::to_underlying(symbol);
        if (idx >= this->symbolRules.size()) {
            return unhandledRule();
        }

        const ScopedSlice slice = this->symbolScopeRanges[idx];
        if (slice.count > 0u) {
            for (uint32_t i = 0; i < slice.count; ++i) {
                const ScopedSymbolRule &entry = this->scopedRules[slice.offset + i];
                const bool cstOk = (entry.cstParent == ANY_CST_SYMBOL) || (entry.cstParent == cstParentSymbol);
                const bool astOk = (entry.astParent == ASTKind::None) || (entry.astParent == astParentKind);
                if (cstOk && astOk) {
                    return entry.rule;
                }
            }
        }

        return this->symbolRules[idx];
    }
    ASTRole LoweringTable::lookupRole(ASTKind parentKind, ts::FieldId fieldId) const noexcept {
        const size_t col = std::to_underlying(fieldId);
        if (col == 0u || col >= this->fieldStride) {
            return ASTRole::None;
        }

        const size_t kindIdx = std::to_underlying(parentKind);
        if (kindIdx > 0u && kindIdx < this->kindToRoleRow.size()) {
            const uint16_t compactRow = this->kindToRoleRow[kindIdx];
            if (compactRow != 0u) {
                const ASTRole scoped = this->roleMatrix[static_cast<size_t>(compactRow) * this->fieldStride + col];
                if (scoped != ASTRole::None) {
                    return scoped;
                }
            }
        }
        return this->roleMatrix[col]; // Row 0: global default role
    }

    const SymbolRule& LoweringTable::unhandledRule() noexcept {
        static constexpr SymbolRule kUnhandled{};
        return kUnhandled;
    }
}
