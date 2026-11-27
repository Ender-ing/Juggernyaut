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

    struct CSTVisitorConfigs {
        bool reportUnusedCSTNodes = false;   // Debug flag
        uint32_t maxSyntaxDiagnostics = 100; // Max syntax diagnostics emitted per `visit` (0 = unlimited)
    };

    class JUG_PARSER_API CSTVisitor {
        private:
            // Non-owning type-erased binding active only during `visitImpl`
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

            // Cached per-depth traversal state (eliminates duplicate `classify`/`isExtra` calls in `onLeave`)
            enum class TraversalTag : uint8_t {
                ExtraOrError    = 0, // Extra trivia/comment leaf or syntax-error node
                ExtraScope      = 1, // Root of an extra node containing inner error recovery tokens
                ValidUnhandled  = 2, // Valid non-extra node; neither consumer nor parent handled it
                ParentInherited = 3, // Valid unnamed token inheriting handled status (`consumerHandled == false`)
                ConsumerHandled = 4 // Valid non-extra node where `consumer.onValidEnter` returned `true`
            };

            struct PendingUnused {
                ts::Node node;
                uint32_t siblingSeq = 0;
            };

            // Unused-node LIFO frame (12B; O(1) contiguous-sibling coalescing via `childCounter`)
            struct UnusedFrame {
                uint32_t pendingStart = 0;
                uint32_t childCounter = 0;
                bool isUnused = false;
                bool hasUsedDescendant = false;
            };

            std::vector<UnusedFrame> unusedStack;
            std::vector<PendingUnused> unusedPendingNodes;
            std::vector<TraversalTag> traversalStack;
            uint32_t extraDepth = 0;
            uint32_t emittedSyntaxDiagnostics = 0;
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
