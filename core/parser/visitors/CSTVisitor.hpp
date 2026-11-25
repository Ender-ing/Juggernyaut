/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// Tree-sitter
#include "../tree-sitter.hpp"

// Host, Diagnostics & AST
#include "../../data/host/VFS.hpp"
#include "../../data/diagnostics/Diagnostic.hpp"
#include "../../data/parser/AST.hpp"

// AST logic
#include "ASTMapper.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Parser::Visitors {
    // Host
    using File = Data::Host::File;

    // Diagnostics
    using Diagnostic = Data::Diagnostics::Diagnostic;
    using ModifiableEvent = Data::Diagnostics::ModifiableEvent;

    // AST
    using AST = Data::Parser::AST;
    using ASTNode = Data::Parser::ASTNode;
    using ASTKind = Data::Parser::ASTKind;
    using ASTRole = Data::Parser::ASTRole;

    // Tree-sitter node state classification
    enum class NodeState : uint8_t {
        Valid, // Standard language CST node.
        Error, // Built-in Tree-sitter `ERROR` recovery span.
        Unexpected, // Leaf `ERROR` node covering unexpected bytes (`(UNEXPECTED ...)`).
        Missing, // Parser-inserted zero-width `MISSING` token.
    };

    struct CSTVisitorConfigs {
        bool reportUnusedCSTNodes = false; // Debug flag
        bool buildAST = true;
    };

    // (via `ts::visit()`)
    class JUG_PARSER_API CSTVisitor {
        private:
            // Tracking unused nodes
            struct UnusedFrame {
                ts::NodeID nodeId{};
                bool isUnused = false;
                bool hasUsedDescendant = false;
                std::vector<ts::Node> pending;
            };
            std::vector<UnusedFrame> unusedStack;
            ts::NodeID extraRootId{};

            // AST builder frame
            struct ASTFrame {
                ts::Node cstNode;
                ts::FieldId cstFieldId = ts::FieldId::None;
                uint32_t reservedSlot = 0;
                bool hasReservedSlot = false;
                ASTNode astNode{};
                std::vector<std::pair<ts::FieldId, ASTNode>> pendingChildren;
            };
            std::vector<bool> handledStack;
            std::vector<ASTFrame> astStack;
            ts::FieldId currentFieldId = ts::FieldId::None;

            // AST
            const ASTMapper *mapper = nullptr;
            std::unique_ptr<AST> ast = nullptr;

            // File details
            const File *file = nullptr;
            const ModifiableEvent *onDiagnostic = nullptr;
            bool syntaxErrors = false;

            // Configs
            CSTVisitorConfigs configs;

            // Internal AST helpers
            void contaminateCurrentASTNode(ts::Node errorNode) ;
            void finalizeASTFrame(ts::Node node, const NodeRule &rule) ;
        protected:
            // Diagnostics
            void emitDiagnostic(Diagnostic &diag) ;
            void emitUnusedNodeDiagnostic(ts::Node first, ts::Node last) ;
            void reportSyntaxError(ts::Node node, NodeState state) ;
            void finalizeUnusedFrame(ts::Node node) ;
            void reportUnusedNode(ts::Node node, NodeState state) ;

            // (#NodeState::Valid).
            virtual ts::VisitAction onNode(ts::Node node) ;
            // (#NodeState::Error)
            virtual ts::VisitAction onError(ts::Node node) ;
            // (#NodeState::Unexpected)
            virtual ts::VisitAction onUnexpected(ts::Node node) ;
            // (#NodeState::Missing)
            virtual ts::VisitAction onMissing(ts::Node node) ;

        public:
            explicit CSTVisitor(const ASTMapper *astMapper = nullptr) noexcept ;
            virtual ~CSTVisitor() = default;

            CSTVisitor(const CSTVisitor&) noexcept = delete;
            CSTVisitor& operator=(const CSTVisitor&) noexcept = delete;
            CSTVisitor(CSTVisitor&&) noexcept = default;
            CSTVisitor& operator=(CSTVisitor&&) noexcept = default;

            void configure(CSTVisitorConfigs configurations, const File *targetFile, const ModifiableEvent *diagnosticCallback) noexcept ;

            // Iteratively visits `tree`, starting at its root node
            void visit(const ts::Tree& tree) ;
            // Iteratively visits the subtree rooted at `root`.
            void visit(ts::Node root) ;

            // Entry hooks (`onEnter(node)` satisfies `ts::TreeVisitor`)
            [[nodiscard]] ts::VisitAction onEnter(ts::Node node) ;
            [[nodiscard]] ts::VisitAction onEnter(ts::Node node, ts::FieldId fieldId) ;
            // Post-order hook required by `ts::LeavingTreeVisitor`
            virtual void onLeave(ts::Node node) ;

            // Static helpers
            [[nodiscard]] static NodeState classify(ts::Node node) noexcept;
            [[nodiscard]] static Diagnostic buildSyntaxDiagnostic(
                ts::Node node,
                std::string_view source = "",
                const File *originFile = nullptr) ;

            // Queries & Output
            [[nodiscard]] bool hasSyntaxErrors() const noexcept ;
            [[nodiscard]] std::unique_ptr<AST> takeAST() noexcept ;
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
