/**
 * @brief
 * Immutable O(1) CST-to-AST symbol & scoped field-role lowering table
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// Tree-sitter
#include "../tree-sitter.hpp"

// AST
#include "../../data/parser/AST.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Parser::Lowering {
    class ASTBuilder;

    // AST
    using ASTNode = Data::Parser::ASTNode;
    using ASTKind = Data::Parser::ASTKind;
    using ASTRole = Data::Parser::ASTRole;

    // Action performed by `ASTBuilder` when encountering a CST symbol
    enum class LoweringOp : uint8_t {
        Unhandled = 0, // Unmapped rule (triggers unused-CST-node hint in debug mode if named)
        Ignore,        // Handled node/token that produces no AST output
        EmitNode,      // Pushes an ASTFrame and emits an ASTNode
        PassThrough,   // Transparent CST wrapper (unwraps children; forwards field if `forwardField` is set)
        MutateParent,  // Stamps `astKind` (if != None), `subKind`, and `flags` onto the enclosing ASTFrame
        MutateNext     // Stages `subKind` and `flags` on the enclosing scope for the next sibling `EmitNode`
    };

    // Per-symbol lowering descriptor (32B on x64 = 2 per cache line: 2B op/pad + 8B tags/flags + 6B bools + 16B hooks)
    struct SymbolRule {
        using Hook = void(*)(ASTBuilder&, ts::Node, ASTNode&);

        LoweringOp op = LoweringOp::Unhandled;
        ASTKind astKind = ASTKind::None;
        ASTKind targetParent = ASTKind::None; // Optional filter for `MutateParent` / `MutateNext` (`None` = any parent)
        uint16_t subKind = 0;
        uint16_t flags = 0;
        bool resolveChildRoles = false;
        bool forwardField = false;
        bool internText = false;       // Interns `sourceText(node)` into `ASTNode::lexemeId` on `EmitNode`
        bool attachToNext = false;     // Stages finalized `EmitNode` as leading child of next sibling `EmitNode`
        bool clobberSubKind = true;    // If false, `MutateParent`/`MutateNext` only writes `subKind` when still 0
        bool runHooksOnError = false;  // If true, still runs `onLeave` when `ASTFlags::HasError` is set
        Hook onEnter = nullptr;
        Hook onLeave = nullptr;
    };

    // Startup validation categories recorded when `bind*` calls fail or collide
    enum class RegistrationIssueKind : uint8_t {
        MissingSymbol,       // `symbolName` (with matching `isNamed`) does not exist in `lang`
        MissingParentSymbol, // `cstParentName` does not exist in `lang`
        MissingField,        // `fieldName` does not exist in `lang`
        DuplicateSymbolRule, // Same `(symbol, cstParent, astParent)` slot was bound more than once
        DuplicateFieldRole   // Same `(parentKind, fieldName)` slot was bound more than once
    };

    // Diagnostic record appended to `LoweringTable::issues` whenever a `bind*` call fails at startup.
    // Allows tests and debug builds to catch grammar rule/field typos and cross-module token collisions
    // immediately instead of silently dropping or clobbering rules.
    struct RegistrationIssue {
        RegistrationIssueKind kind = RegistrationIssueKind::MissingSymbol;
        std::string targetName;             // Symbol or field name being registered
        std::string scopeName;              // Optional `cstParentName` context (empty if unscoped by CST rule)
        ASTKind parentKind = ASTKind::None; // Optional `ASTKind` context (`None` if unscoped by AST parent)
    };

    class JUG_PARSER_API LoweringTable {
        public:
            using RawSymbol = std::underlying_type_t<ts::Symbol>;
            static constexpr ts::Symbol ANY_CST_SYMBOL = static_cast<ts::Symbol>(~RawSymbol{0});

        private:
            // Half-open slice `[offset, offset + count)` into `scopedRules` for a given `ts::Symbol`.
            // Unambiguous symbols (>90% of grammar rules/tokens) keep `count == 0`, allowing `lookupSymbol`
            // to return `symbolRules[symbol]` in a single direct array read without touching `scopedRules`.
            struct ScopedSlice {
                uint32_t offset = 0;
                uint16_t count = 0;
            };

            // Context-sensitive `SymbolRule` override stored contiguously per symbol inside `scopedRules`.
            // Matches when the node's immediate CST parent equals `cstParent` (`ANY_CST_SYMBOL` = wildcard)
            // AND its enclosing `ASTFrame` kind equals `astParent` (`ASTKind::None` = wildcard).
            // Entries within each symbol's `ScopedSlice` are kept sorted from most-specific
            // `(cstParent + astParent)` -> `cstParent`-only -> `astParent`-only so the first match wins.
            struct ScopedSymbolRule {
                ts::Symbol cstParent = ANY_CST_SYMBOL;
                ASTKind astParent = ASTKind::None;
                SymbolRule rule{};
            };

            ts::Language lang{nullptr};
            uint32_t fieldStride = 0;                    // `numFields + 1` (column count in `roleMatrix`)
            uint16_t roleRows = 0;                       // Compact active row count in `roleMatrix` (row 0 = global default)
            std::vector<SymbolRule> symbolRules;         // Default unscoped rule indexed by `ts::Symbol` (named + anonymous)
            std::vector<ScopedSlice> symbolScopeRanges;  // Per-`ts::Symbol` contiguous slice into `scopedRules`
            std::vector<ScopedSymbolRule> scopedRules;   // Flat storage for context-scoped symbol rule overrides
            std::vector<uint16_t> kindToRoleRow;         // Sparse-safe `ASTKind -> compact row` map (`0` = use row 0 default)
            std::vector<ASTRole> roleMatrix;             // Flat `[compactRow * fieldStride + FieldId]` role table
            std::vector<RegistrationIssue> issues;       // Startup validation issues (empty on clean table build)

            void resolveSymbols(std::string_view name, bool isNamed, std::vector<ts::Symbol> &out) const;
            bool insertScopedSymbolRule(ts::Symbol sym, ts::Symbol cstParent,
                ASTKind astParent, const SymbolRule &rule, std::string_view debugSymName,
                std::string_view debugScopeName) ;

        public:
            // Pre-allocates symbol and default-role tables from `language`.
            // Build once, verify `!hasRegistrationIssues()`, and share read-only across threads.
            explicit LoweringTable(ts::Language language) ;
            ~LoweringTable() = default;

            LoweringTable(const LoweringTable&) = default;
            LoweringTable& operator=(const LoweringTable&) = default;
            LoweringTable(LoweringTable&&) noexcept = default;
            LoweringTable& operator=(LoweringTable&&) noexcept = default;

            // --- Symbol Registration API ---
            // All `bind*` methods resolve every matching `ts::Symbol` ID in `lang` (including grammar aliases).
            // Returns `false` and records a `RegistrationIssue` if a symbol/parent is absent from `lang`
            // or if the exact same `(symbol, cstParent, astParent)` slot was already bound.

            // Low-level primitive: binds any named rule (`isNamed = true`) or anonymous literal token
            // (`isNamed = false`) to a default unscoped `SymbolRule`. Prefer the specialized `bind*`
            // helpers below unless combining options they don't expose.
            bool bindSymbol(std::string_view symbolName, SymbolRule rule, bool isNamed = true) ;
            // Context-sensitive primitive: binds `symbolName` only when encountered inside immediate
            // CST parent `cstParentName` (if non-empty) and/or enclosing `ASTFrame` kind `astParent`
            // (if `!= ASTKind::None`). Lookup precedence: `(cstParent + astParent)` > `cstParent` >
            // `astParent` > unscoped `bindSymbol` fallback.
            bool bindScopedSymbol(std::string_view symbolName, SymbolRule rule,
                bool isNamed, std::string_view cstParentName, ASTKind astParent = ASTKind::None,
                bool cstParentIsNamed = true);
            // Shorthand for `bindScopedSymbol` keyed strictly by immediate named CST parent rule
            // (e.g. disambiguating anonymous tokens inside `PassThrough` wrappers).
            bool bindSymbolInCSTParent(std::string_view cstParentName, std::string_view symbolName,
                SymbolRule rule, bool isNamed = false) ;
            // Shorthand for `bindScopedSymbol` keyed strictly by enclosing `ASTFrame` kind.
            bool bindSymbolForParentKind(ASTKind astParent, std::string_view symbolName,
                SymbolRule rule, bool isNamed = false) ;
            // Standard 1-to-1 mapping (`EmitNode`): lowers a named CST rule into an `ASTNode` of `kind`.
            // Set `resolveChildRoles = true` when the grammar rule uses `field(...)` labels on children;
            // supply `onEnter`/`onLeave` only when custom side-effects (e.g. `pushImport`) are needed.
            bool bindNode(std::string_view ruleName, ASTKind kind, bool resolveChildRoles = false,
                SymbolRule::Hook onEnter = nullptr, SymbolRule::Hook onLeave = nullptr) ;
            // Full-descriptor `EmitNode` overload for named rules that also need initial `subKind`,
            // `flags`, or other `SymbolRule` options pre-populated (e.g. `type_alias` vs `abstract_alias`
            // sharing `ASTKind::TypeDecl`).
            bool bindNode(std::string_view ruleName, SymbolRule rule) ;
            // Terminal/leaf `EmitNode` mapping (e.g. `identifier`, `lit_int`, `lit_string`).
            // When `internText = true` (default), interns `sourceText(node)` into `AST` during lowering
            // and stores the deduplicated `LexemeId` on `ASTNode::lexemeId` for O(1) equality downstream.
            bool bindLeaf(std::string_view ruleName, ASTKind kind, bool internText = true,
                SymbolRule::Hook onLeave = nullptr) ;
            // Sibling-prefix `EmitNode` mapping (`attachToNext = true`, e.g. `directive` `@inline` in
            // `_global` / `_generic_content`). Emits an `ASTNode` as usual, then stages it on the
            // enclosing scope so the next non-prefix sibling `EmitNode` claims it as a leading child
            // instead of leaving it detached in `Program` or `Block`.
            bool bindPrefixNode(std::string_view ruleName, ASTKind kind,
                bool resolveChildRoles = false, SymbolRule::Hook onEnter = nullptr,
                SymbolRule::Hook onLeave = nullptr) ;
            // Flattens an intermediate named CST rule (`PassThrough`) so its children attach directly
            // to the enclosing parent `ASTFrame` without creating a wrapper `ASTNode`:
            // - `forwardField = true`: use on single-value or homogeneous list wrappers (e.g.
            //   `expr_grouped` `(x)`, `import_source`, `import_list`) so an outer CST field like
            //   `condition: (expr_grouped (a + b))` forwards onto `a + b`.
            // - `forwardField = false`: use on multi-child structural groups (e.g. `function_details`,
            //   `op_assign`) so the wrapper's own outer field does not leak onto unfielded inner children.
            bool bindPassThrough(std::string_view ruleName, bool forwardField = false,
                SymbolRule::Hook onEnter = nullptr, SymbolRule::Hook onLeave = nullptr) ;
            // Binds a child token/rule (`MutateParent`, defaults to anonymous tokens `isNamed = false`)
            // that stamps `subKind`, ORs `flags`, and optionally rewrites `kind = overrideKind` on the
            // enclosing `ASTFrame` without emitting a child node (e.g. `+=`, `const`, `while`).
            // - When `targetParent != ASTKind::None`, registers as an `astParent`-scoped rule so tokens
            //   reused across distinct constructs (e.g. `-` in `UnaryExpr` vs `BinaryExpr`, or `<` in
            //   `ComparisonExpr` vs generic `TypeSpec<T>`) coexist without overwriting each other.
            // - Set `clobberSubKind = false` when an earlier token in the same rule must take priority
            //   (e.g. `while` after `do` in `do { ... } while (cond);`).
            bool bindParentModifier(std::string_view tokenName, uint16_t subKind,
                uint16_t flags = 0, ASTKind targetParent = ASTKind::None,
                ASTKind overrideKind = ASTKind::None, bool isNamed = false,
                bool clobberSubKind = true) ;
            // Same as `bindParentModifier`, but scoped to immediate named CST parent `cstParentName`.
            // Use when two `PassThrough` CST wrappers share both the same token and the same enclosing
            // `ASTKind` (e.g. `++` / `--` inside `op_prefix` vs `op_postfix` under `UnaryExpr`).
            bool bindCSTParentModifier(std::string_view cstParentName, std::string_view tokenName,
                uint16_t subKind, uint16_t flags = 0, ASTKind targetParent = ASTKind::None,
                ASTKind overrideKind = ASTKind::None, bool isNamed = false,
                bool clobberSubKind = true) ;
            // Binds a prefix token/rule (`MutateNext`, defaults to `isNamed = false`) that stages
            // `subKind` and `flags` on the enclosing scope to be consumed by the very next non-prefix
            // sibling `EmitNode` (e.g. `export` before `_declaration` in `_global`, scoped with
            // `cstParentName = "prog"` so `export` inside `stat_bind` or `export_discrete` remains
            // free to use `bindCSTParentModifier` / `bindIgnore`).
            bool bindPrefixModifier(std::string_view tokenName, uint16_t subKind,
                uint16_t flags = 0, std::string_view cstParentName = {},
                bool isNamed = false, bool clobberSubKind = true) ;
            // Marks a named rule or token as intentionally handled with zero AST output (`Ignore`).
            // Use for valid CST rules with no AST representation (e.g. no-op/empty constructs) so debug
            // `reportUnusedCSTNodes` does not flag them. Unnamed delimiters (`;`, `{`, `}`, `,`) inside
            // already-handled parent rules inherit handled status automatically and don't need this.
            bool bindIgnore(std::string_view symbolName, bool isNamed = true) ;

            // --- Field Role Registration API ---
            // Returns `false` and records a `RegistrationIssue` if `fieldName` is absent from `lang`
            // or if the same `(parentKind, fieldName)` slot was already bound.

            // Registers a language-wide fallback `fieldName -> role` mapping (row 0 of `roleMatrix`).
            // Use when a Tree-sitter field name has one consistent meaning everywhere (`"name"`, `"body"`, `"condition"`).
            bool bindDefaultRole(std::string_view fieldName, ASTRole role) ;
            // Registers a parent-specific `(parentKind, fieldName) -> role` override that takes precedence
            // over `bindDefaultRole` when finalizing an `ASTFrame` of `parentKind` (allocating compact
            // `roleMatrix` rows only for `ASTKind`s that actually register overrides).
            // Use when different rules reuse the same field name for distinct roles (e.g. `"value"` or `"target"`).
            bool bindScopedRole(ASTKind parentKind, std::string_view fieldName, ASTRole role) ;

            // --- Registration Validation ---
            // Inspect or assert `!hasRegistrationIssues()` after populating the table at startup.
            [[nodiscard]] bool hasRegistrationIssues() const noexcept;
            [[nodiscard]] std::span<const RegistrationIssue> registrationIssues() const noexcept;

            // --- Hot-Path O(1) Lookups (used by `ASTBuilder` during traversal) ---
            // Returns the default unscoped `SymbolRule` bound to `symbol`, or `unhandledRule()` if unbound.
            [[nodiscard]] const SymbolRule& lookupSymbol(ts::Symbol symbol) const noexcept;
            // Resolves `symbol` against `(cstParentSymbol, astParentKind)` scoped overrides first,
            // falling back to the unscoped rule in `symbolRules[symbol]` or `unhandledRule()`.
            [[nodiscard]] const SymbolRule& lookupSymbol( ts::Symbol symbol,
                ts::Symbol cstParentSymbol, ASTKind astParentKind) const noexcept;
            // Resolves `(parentKind, fieldId)` via scoped override first, falling back to `bindDefaultRole`.
            [[nodiscard]] ASTRole lookupRole(ASTKind parentKind, ts::FieldId fieldId) const noexcept;

            // Shared static sentinel returned for unmapped symbols (`op == LoweringOp::Unhandled`).
            [[nodiscard]] static const SymbolRule& unhandledRule() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
