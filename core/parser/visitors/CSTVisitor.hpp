/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#pragma once

#include "common/headers.hpp"
#include "../dynamic.hpp" // JUG_PARSER_API

// Tree-sitter
#include "../tree-sitter.hpp"

// Host & Diagnostics
#include "../../data/host/VFS.hpp"
#include "../../data/diagnostics/Diagnostic.hpp"

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

    // Tree-sitter node state classification
    enum class NodeState : uint8_t {
        Valid, // Standard language CST node.
        Error, // Built-in Tree-sitter `ERROR` recovery span.
        Unexpected, // Leaf `ERROR` node covering unexpected bytes (`(UNEXPECTED ...)`).
        Missing, // Parser-inserted zero-width `MISSING` token.
    };

    struct CSTVisitorConfigs {
        bool reportUnusedCSTNodes = false; // Debug flag
    };

    // (via `ts::visit()`)
    class JUG_PARSER_API CSTVisitor {
        private:
            // Tracking unused nodes
            struct UnusedFrame {
                const void *nodeId = nullptr;
                bool isUnused = false;
                bool hasUsedDescendant = false;
                std::vector<ts::Node> pending;
            };
            std::vector<UnusedFrame> unusedStack;
            const void *extraRootId = nullptr;

            // File details
            const File *file = nullptr;
            ModifiableEvent onDiagnostic;
            bool syntaxErrors = false;

            // Configs
            CSTVisitorConfigs configs;

        protected:
            // Diagnostics
            void emitDiagnostic(Diagnostic &diag) ;
            void emitUnusedNodeDiagnostic(ts::Node first, ts::Node last) ;
            void reportSyntaxError(ts::Node node, NodeState state) ;
            void finalizeUnusedFrame(ts::Node node) ;
            void reportUnusedNode(ts::Node node, NodeState state) ;

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
            CSTVisitor(CSTVisitorConfigs configurations, const File *targetFile = nullptr, ModifiableEvent diagnosticCallback = nullptr) noexcept
                : file(targetFile), onDiagnostic(std::move(diagnosticCallback)), configs(std::move(configurations)) {}
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
            // Builds a concise, pinpoint syntax Diagnostic for an `Error`, `Unexpected`, or `Missing` node
            [[nodiscard]] static Diagnostic buildSyntaxDiagnostic(
                ts::Node node,
                std::string_view source = "",
                const File *originFile = nullptr) ;

            // Queries
            [[nodiscard]] bool hasSyntaxErrors() const noexcept { return this->syntaxErrors; }
    };

}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
