/**
 * @brief 
 * Tree-sitter CST walker, syntax error reporter, and CSTConsumer driver
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
        Valid,      // Standard language CST node.
        Error,      // Built-in Tree-sitter `ERROR` recovery span.
        Unexpected, // Leaf `ERROR` node covering unexpected bytes (`(UNEXPECTED ...)`).
        Missing     // Parser-inserted zero-width `MISSING` token.
    };

    // Downstream sink notified of valid CST nodes and syntax error spans during `CSTVisitor::visit`
    template <typename C>
    concept CSTConsumer = requires(C &consumer, ts::Node node, ts::FieldId fieldId) {
        { consumer.onValidEnter(node, fieldId) } -> std::same_as<bool>;
        { consumer.onValidLeave(node) } -> std::same_as<void>;
        { consumer.onSyntaxError(node) } -> std::same_as<void>; // Watch out: The visitor already reports syntax errors!
    };

    // Per-walk visitor behavior and diagnostic limits
    struct CSTVisitorConfigs {
        // Debug flag: emits hint diagnostics on valid named CST subtrees rejected by the consumer.
        bool reportUnusedCSTNodes = false;
        // Caps how many rich syntax-error `Diagnostic` objects are built/emitted per `visit` (`0` = unlimited).
        // Prevents error-cascade floods (and repeated `TSLookaheadIterator` allocations) on broken files
        // while still notifying `consumer.onSyntaxError` for every error span so AST error flags stay exact.
        uint32_t maxSyntaxDiagnostics = 100;
    };

    class JUG_PARSER_API CSTVisitor {
        private:
            // Non-owning type-erased binding to a concrete `CSTConsumer`, active only during `visitImpl`.
            // Keeps `CSTVisitor`'s traversal implementation compiled once inside the parser library.
            struct ConsumerBridge {
                void *ctx = nullptr;
                bool (*onValidEnter)(void*, ts::Node, ts::FieldId) = nullptr;
                void (*onValidLeave)(void*, ts::Node) = nullptr;
                void (*onSyntaxError)(void*, ts::Node) = nullptr;

                template <CSTConsumer C>
                [[nodiscard]] static ConsumerBridge bind(C &target) noexcept {
                    return ConsumerBridge{
                        .ctx = std::addressof(target),
                        .onValidEnter = [](void *p, ts::Node n, ts::FieldId fid) -> bool {
                            return static_cast<C*>(p)->onValidEnter(n, fid);
                        },
                        .onValidLeave = [](void *p, ts::Node n) -> void {
                            static_cast<C*>(p)->onValidLeave(n);
                        },
                        .onSyntaxError = [](void *p, ts::Node n) -> void {
                            static_cast<C*>(p)->onSyntaxError(n);
                        },
                    };
                }
            };

            // 1-byte classification pushed onto `traversalStack` in `onEnter` and popped in `onLeave`.
            // Records how each CST node was classified and handled on entry so `onLeave` can:
            //   1. Avoid re-running `classify(node)` and `node.isExtra()` C-API queries on exit, and
            //   2. Skip calling `consumer.onValidLeave` on unhandled tokens (e.g. `;`, `,`, `(`, `)`).
            enum class TraversalTag : uint8_t {
                ExtraOrError    = 0, // Trivia/comment leaf or syntax-error node (no consumer leave or unused frame).
                ExtraScope      = 1, // Root of an `extra` node that contains inner recovery errors (`extraDepth` reset on leave).
                ValidUnhandled  = 2, // Valid non-extra node rejected by both `consumer.onValidEnter` and its parent.
                ParentInherited = 3, // Valid unnamed token (`;`, `{`, `,`) inheriting handled status from its parent rule.
                ConsumerHandled = 4, // Valid non-extra node accepted by `consumer.onValidEnter` (requires `onValidLeave`).
            };

            // Collapsed unhandled child CST node staged in `unusedPendingNodes` when `reportUnusedCSTNodes` is active.
            // Pairs the CST `node` with its 1-based non-extra `siblingSeq` under its parent so `finalizeUnusedFrame`
            // can coalesce adjacent unhandled siblings via `curr.siblingSeq == prevSeq + 1` in O(1) instead of
            // calling `ts::Node::getNextSibling()` (which walks from the CST root and turns large blocks into O(N^2)),
            // while naturally bridging across interspersed comments.
            struct PendingUnused {
                ts::Node node{TSNode{}};
                uint32_t siblingSeq = 0; // 1-based position among non-extra direct children of the parent frame
            };

            // Per-node LIFO frame on `unusedStack` (12B, trivially copyable), active only when
            // `reportUnusedCSTNodes` is enabled. Tracks whether an unhandled subtree can collapse into a
            // single parent diagnostic or must emit coalesced runs of its unhandled children.
            struct UnusedFrame {
                uint32_t pendingStart = 0;      // Start index of this node's slice in `unusedPendingNodes`
                uint32_t childCounter = 0;      // Monotonic count of non-extra direct children (generates `siblingSeq`)
                bool isUnused = false;          // True if this node itself was not handled
                bool hasUsedDescendant = false; // True if any descendant was handled or contained a syntax error
            };

            // Unused-node debug tracking buffers (empty & untouched during normal parses)
            std::vector<UnusedFrame> unusedStack;
            std::vector<PendingUnused> unusedPendingNodes;

            // Active traversal state
            std::vector<TraversalTag> traversalStack; // Parallel to active cursor depth (1 entry per entered CST node)
            uint32_t extraDepth = 0;                  // Non-zero while descending inside an error-bearing `extra` subtree
            uint32_t emittedSyntaxDiagnostics = 0;    // Count of syntax `Diagnostic`s emitted so far in current `visit`
            ts::FieldId currentFieldId = ts::FieldId::None;
            ConsumerBridge consumer{};

            // File & diagnostic state
            const File *file = nullptr;
            const ModifiableEvent *onDiagnostic = nullptr;
            bool syntaxErrors = false;

            // Configs
            CSTVisitorConfigs configs{};

            void resetState(bool trimExcessCapacity) noexcept;
            void visitImpl(const ts::Tree &tree, ConsumerBridge boundConsumer) ;
            void visitImpl(ts::Node root, ConsumerBridge boundConsumer) ;

        protected:
            // Diagnostics
            void emitDiagnostic(Diagnostic &diag) ;
            void emitUnusedNodeDiagnostic(ts::Node first, ts::Node last) ;
            void reportSyntaxError(ts::Node node, NodeState state) ;
            void finalizeUnusedFrame(ts::Node node) ;
            void reportUnusedNode(ts::Node node, NodeState state) ;

            // Node dispatch hooks
            virtual ts::VisitAction onNode(ts::Node node) ;
            virtual ts::VisitAction onError(ts::Node node) ;
            virtual ts::VisitAction onUnexpected(ts::Node node) ;
            virtual ts::VisitAction onMissing(ts::Node node) ;

        public:
            CSTVisitor() noexcept = default;
            virtual ~CSTVisitor() = default;

            CSTVisitor(const CSTVisitor&) noexcept = delete;
            CSTVisitor& operator=(const CSTVisitor&) noexcept = delete;
            CSTVisitor(CSTVisitor&&) noexcept = default;
            CSTVisitor& operator=(CSTVisitor&&) noexcept = default;

            void configure(CSTVisitorConfigs configurations, const File *targetFile,
                const ModifiableEvent *diagnosticCallback) noexcept;

            // Standalone CST walk (syntax & unused-node diagnostics only)
            void visit(const ts::Tree &tree) ;
            void visit(ts::Node root) ;

            // Consumer-driven CST walk
            template <CSTConsumer C>
            void visit(const ts::Tree &tree, C &targetConsumer) {
                this->visitImpl(tree, ConsumerBridge::bind(targetConsumer));
            }
            template <CSTConsumer C>
            void visit(ts::Node root, C &targetConsumer) {
                this->visitImpl(root, ConsumerBridge::bind(targetConsumer));
            }

            // Entry & post-order hooks (`ts::FieldTreeVisitor` / `ts::LeavingTreeVisitor`)
            [[nodiscard]] ts::VisitAction onEnter(ts::Node node) ;
            [[nodiscard]] ts::VisitAction onEnter(ts::Node node, ts::FieldId fieldId) ;
            virtual void onLeave(ts::Node node) ;

            // Static helpers
            [[nodiscard]] static NodeState classify(ts::Node node) noexcept;
            [[nodiscard]] static Diagnostic buildSyntaxDiagnostic(ts::Node node,
                std::string_view source = "", const File *originFile = nullptr) ;

            // Queries
            [[nodiscard]] bool hasSyntaxErrors() const noexcept;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
