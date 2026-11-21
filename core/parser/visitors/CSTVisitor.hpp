/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

#include "../tree-sitter.hpp"

namespace Parser::Visitors {

    // Tree-sitter node state classification
    enum class NodeState : uint8_t {
        Valid, // Standard language CST node.
        Error, // Built-in Tree-sitter `ERROR` recovery span.
        Unexpected, // Leaf `ERROR` node covering unexpected bytes (`(UNEXPECTED ...)`).
        Missing, // Parser-inserted zero-width `MISSING` token.
    };

    // (via `ts::visit()`)
    class JUG_PARSER_API CSTVisitor {
        protected:
            // (#NodeState::Valid).
            // @return `ts::VisitAction::Continue` by default
            virtual ts::VisitAction onNode(ts::Node node) ;

            // Called upon entering a compound/recovery `ERROR` node (#NodeState::Error)
            // @return `ts::VisitAction::SkipChildren` by default
            virtual ts::VisitAction onError(ts::Node node) ;

            // Called upon entering a leaf `UNEXPECTED` error node (#NodeState::Unexpected)
            // @return `ts::VisitAction::SkipChildren` by default
            virtual ts::VisitAction onUnexpected(ts::Node node) ;

            // Called upon entering a parser-synthesized `MISSING` node (#NodeState::Missing)
            // @return `ts::VisitAction::SkipChildren` by default
            virtual ts::VisitAction onMissing(ts::Node node) ;

        public:
            CSTVisitor() noexcept = default;
            virtual ~CSTVisitor() = default;

            CSTVisitor(const CSTVisitor&) noexcept = default;
            CSTVisitor& operator=(const CSTVisitor&) noexcept = default;
            CSTVisitor(CSTVisitor&&) noexcept = default;
            CSTVisitor& operator=(CSTVisitor&&) noexcept = default;

            // Iteratively visits `tree`, starting at its root node
            void visit(const ts::Tree& tree) ;
            // Iteratively visits the subtree rooted at `root`.
            void visit(ts::Node root) ;

            // Entry hook required by `ts::TreeVisitor`; routes `node` by its #NodeState
            // @return Traversal directive (`Continue`, `SkipChildren`, or `Stop`)
            [[nodiscard]] ts::VisitAction onEnter(ts::Node node) ;
            // Post-order hook required by `ts::LeavingTreeVisitor` (no-op by default)
            virtual void onLeave(ts::Node node) ;

            // Classifies `node` as `Valid`, `Error`, `Unexpected`, or `Missing`.
            // @return #NodeState of `node` (`NodeState::Error` if `node.isNull()`)
            [[nodiscard]] static NodeState classify(ts::Node node) noexcept;
    };

}
