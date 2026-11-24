/**
 * @brief
 * Declarative CST-to-AST symbol & field role mapping table
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

namespace Parser::Visitors {
    class CSTVisitor;

    // AST
    using ASTNode = Data::Parser::ASTNode;
    using ASTKind = Data::Parser::ASTKind;
    using ASTRole = Data::Parser::ASTRole;

    struct NodeRule {
        using Hook = void(*)(CSTVisitor&, ts::Node, ASTNode&);

        bool handled = false;
        bool resolveChildRoles = false;
        ASTKind astKind = ASTKind::None;
        Hook onEnterHook = nullptr;
        Hook onLeaveHook = nullptr;
    };

    class JUG_PARSER_API ASTMapper {
        private:
            ts::Language lang{nullptr};
            std::vector<NodeRule> symbolRules;
            std::vector<ASTRole> fieldRoles;

        public:
            explicit ASTMapper(ts::Language language) ;
            ~ASTMapper() = default;

            ASTMapper(const ASTMapper&) = default;
            ASTMapper& operator=(const ASTMapper&) = default;
            ASTMapper(ASTMapper&&) noexcept = default;
            ASTMapper& operator=(ASTMapper&&) noexcept = default;

            // Registration API
            // (returns `false` if the name does not exist in `lang`)
            bool bindNode(std::string_view cstRuleName, NodeRule rule, bool isNamed = true) ;
            bool bindField(std::string_view cstFieldName, ASTRole role) ;

            // table lookups
            // (O(1) - used by CSTVisitor)
            [[nodiscard]] const NodeRule& lookupRule(ts::Symbol symbol) const noexcept ;
            [[nodiscard]] ASTRole lookupFieldRole(ts::FieldId fieldId) const noexcept ;
            [[nodiscard]] ASTRole lookupFieldRole(std::string_view fieldName) const noexcept;
            [[nodiscard]] ASTRole resolveChildRole(ts::Node childNode) const noexcept;

            [[nodiscard]] static const NodeRule& unhandledRule() noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
