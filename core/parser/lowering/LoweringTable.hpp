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
    };

    // Per-symbol lowering descriptor (24B on x64: 7B tags/flags + 1B pad + 16B hook pointers)
    struct SymbolRule {
        using Hook = void(*)(ASTBuilder&, ts::Node, ASTNode&);

        LoweringOp op = LoweringOp::Unhandled;
        ASTKind astKind = ASTKind::None;
        ASTKind targetParent = ASTKind::None; // Optional filter for `MutateParent` (`None` = any parent)
        uint8_t subKind = 0;
        uint8_t flags = 0;
        bool resolveChildRoles = false;
        bool forwardField = false;
        Hook onEnter = nullptr;
        Hook onLeave = nullptr;
    };

    class JUG_PARSER_API LoweringTable {
        private:
            ts::Language lang{nullptr};
            uint32_t fieldStride = 0;            // `numFields + 1` (column count in `roleMatrix`)
            uint32_t roleRows = 0;               // Active `ASTKind` row count in `roleMatrix` (row 0 = global default)
            std::vector<SymbolRule> symbolRules; // Indexed by `ts::Symbol` (named + anonymous)
            std::vector<ASTRole> roleMatrix;     // Flat `[ASTKind * fieldStride + FieldId]` table

        public:
            // Pre-allocates symbol and default-role tables from `language`.
            // Build once (e.g. as a `static const` singleton) and share read-only across threads.
            explicit LoweringTable(ts::Language language) ;
            ~LoweringTable() = default;

            LoweringTable(const LoweringTable&) = default;
            LoweringTable& operator=(const LoweringTable&) = default;
            LoweringTable(LoweringTable&&) noexcept = default;
            LoweringTable& operator=(LoweringTable&&) noexcept = default;

            // --- Symbol Registration API (all return `false` if the symbol name is absent from `lang`) ---

            // Low-level primitive: binds any named rule (`isNamed = true`) or anonymous literal token
            // (`isNamed = false`) to a raw `SymbolRule`. Prefer the specialized `bind*` helpers below
            // unless combining options they don't expose (e.g. `MutateParent` with a custom hook).
            bool bindSymbol(std::string_view symbolName, SymbolRule rule, bool isNamed = true) ;

            // Standard 1-to-1 mapping (`EmitNode`): lowers a named CST rule into an `ASTNode` of `kind`.
            // Set `resolveChildRoles = true` when the grammar rule uses `field(...)` labels on children;
            // supply `onEnter`/`onLeave` only when custom side-effects (e.g. `pushImport`) are needed.
            bool bindNode(std::string_view ruleName, ASTKind kind,
                bool resolveChildRoles = false, SymbolRule::Hook onEnter = nullptr,
                SymbolRule::Hook onLeave = nullptr) ;

            // Full-descriptor `EmitNode` overload for named rules that also need initial `subKind` or
            // `flags` pre-populated (e.g. `type_alias` vs `abstract_alias` sharing `ASTKind::TypeDecl`).
            bool bindNode(std::string_view ruleName, SymbolRule rule) ;

            // Flattens an intermediate named CST rule (`PassThrough`) so its children attach directly
            // to the enclosing parent `ASTFrame` without creating a wrapper `ASTNode`:
            // - `forwardField = true`: use on single-value wrappers (e.g. `expr_grouped` `(x)`, `import_source`)
            //   so an outer CST field like `condition: (expr_grouped (a + b))` forwards onto `a + b`.
            // - `forwardField = false`: use on multi-child structural groups (e.g. `function_details`, `op_assign`)
            //   so the wrapper's own outer field does not leak onto unfielded inner children.
            bool bindPassThrough(std::string_view ruleName, bool forwardField = false,
                                 SymbolRule::Hook onEnter = nullptr, SymbolRule::Hook onLeave = nullptr) ;

            // Binds a child token/rule (`MutateParent`, defaults to anonymous tokens `isNamed = false`)
            // that stamps `subKind`, ORs `flags`, and optionally rewrites `kind = overrideKind` on the
            // enclosing `ASTFrame` without emitting a child node (e.g. `+=`, `const`, `export`, `while`).
            // Always set `targetParent` when a token is shared with non-operator constructs (e.g. `<` in
            // `ComparisonExpr` vs generic `TypeSpec<T>`).
            bool bindParentModifier(std::string_view tokenName, uint8_t subKind,
                uint8_t flags = 0, ASTKind targetParent = ASTKind::None,
                ASTKind overrideKind = ASTKind::None, bool isNamed = false) ;

            // Marks a named rule or token as intentionally handled with zero AST output (`Ignore`).
            // Use for valid CST rules with no AST representation (e.g. no-op/empty constructs) so debug
            // `reportUnusedCSTNodes` does not flag them. Unnamed delimiters (`;`, `{`, `}`, `,`) inside
            // already-handled parent rules inherit handled status automatically and don't need this.
            bool bindIgnore(std::string_view symbolName, bool isNamed = true) ;

            // --- Field Role Registration API (returns `false` if `fieldName` is absent from `lang`) ---

            // Registers a language-wide fallback `fieldName -> role` mapping (row 0 of `roleMatrix`).
            // Use when a Tree-sitter field name has one consistent meaning everywhere (`"name"`, `"body"`, `"condition"`).
            bool bindDefaultRole(std::string_view fieldName, ASTRole role);

            // Registers a parent-specific `(parentKind, fieldName) -> role` override that takes precedence
            // over `bindDefaultRole` when finalizing an `ASTFrame` of `parentKind`.
            // Use when different rules reuse the same field name for distinct roles (e.g. `"value"` or `"target"`).
            bool bindScopedRole(ASTKind parentKind, std::string_view fieldName, ASTRole role);

            // --- Hot-Path O(1) Lookups (used by `ASTBuilder` during traversal) ---

            // Returns the `SymbolRule` bound to `symbol`, or `unhandledRule()` if unbound/out-of-range.
            [[nodiscard]] const SymbolRule& lookupSymbol(ts::Symbol symbol) const noexcept;

            // Resolves `(parentKind, fieldId)` via scoped override first, falling back to `bindDefaultRole`.
            [[nodiscard]] ASTRole lookupRole(ASTKind parentKind, ts::FieldId fieldId) const noexcept;

            // Shared static sentinel returned for unmapped symbols (`op == LoweringOp::Unhandled`).
            [[nodiscard]] static const SymbolRule& unhandledRule() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
