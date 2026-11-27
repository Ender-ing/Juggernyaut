/**
 * @brief 
 * Tree-sitter CST walker, syntax error reporter, and CSTConsumer driver
**/

#include "CSTVisitor.hpp"

#include "../internal/cstDiagnostics.hpp"

namespace Parser::Visitors {
    namespace Helper {
        constexpr size_t MAX_RETAINED_STACK_CAPACITY = 4096u;
        constexpr size_t MAX_RETAINED_PENDING_CAPACITY = 16384u;

        template <typename T>
        void resetBuffer(std::vector<T> &vec, bool trimExcess, size_t maxRetainedCap) noexcept {
            vec.clear();
            if (trimExcess && vec.capacity() > maxRetainedCap) {
                std::vector<T>{}.swap(vec);
            }
        }
    }

    void CSTVisitor::resetState(bool trimExcessCapacity) noexcept {
        Helper::resetBuffer(this->unusedStack, trimExcessCapacity, Helper::MAX_RETAINED_STACK_CAPACITY);
        Helper::resetBuffer(this->unusedPendingNodes, trimExcessCapacity, Helper::MAX_RETAINED_PENDING_CAPACITY);
        Helper::resetBuffer(this->traversalStack, trimExcessCapacity, Helper::MAX_RETAINED_STACK_CAPACITY);
        this->extraDepth = 0u;
        this->emittedSyntaxDiagnostics = 0u;
        this->currentFieldId = ts::FieldId::None;
        this->consumer = ConsumerBridge{};
    }
    void CSTVisitor::visitImpl(const ts::Tree &tree, ConsumerBridge boundConsumer) {
        this->visitImpl(tree.getRootNode(), boundConsumer);
    }
    void CSTVisitor::visitImpl(ts::Node root, ConsumerBridge boundConsumer) {
        struct VisitScopeGuard {
            CSTVisitor *self;
            ~VisitScopeGuard() noexcept {
                self->resetState(true);
            }
        } guard{this};

        this->resetState(false);
        this->consumer = boundConsumer;
        this->syntaxErrors = false;

        if (!root.isNull()) {
            ts::visitWithFields(root, *this);
        }
    }

    // Diagnostics
    void CSTVisitor::emitDiagnostic(Diagnostic &diag) {
        diag.file = this->file;
        if (this->onDiagnostic != nullptr) {
            (*this->onDiagnostic)(diag);
        }
    }
    void CSTVisitor::emitUnusedNodeDiagnostic(ts::Node first, ts::Node last) {
        if (this->onDiagnostic == nullptr || !this->configs.reportUnusedCSTNodes) {
            return;
        }

        Diagnostic diag = Internal::CSTDiagnostics::buildRangedDiagnostic(first, last, this->file);
        USE_CODE(diag, Hint, 200009);
        diag.isUnnecessary = true;
        this->emitDiagnostic(diag);
    }
    void CSTVisitor::reportSyntaxError(ts::Node node, NodeState state) {
        if (state == NodeState::Valid) {
            return;
        }

        this->syntaxErrors = true;
        if (this->consumer.onSyntaxError != nullptr) {
            this->consumer.onSyntaxError(this->consumer.ctx, node);
        }
        if (this->onDiagnostic == nullptr) {
            return;
        }
        if (this->configs.maxSyntaxDiagnostics > 0u &&
            this->emittedSyntaxDiagnostics >= this->configs.maxSyntaxDiagnostics) {
            return;
        }
        ++this->emittedSyntaxDiagnostics;

        const std::string_view source = this->file != nullptr ? this->file->content.view() : std::string_view{};
        Diagnostic diag = buildSyntaxDiagnostic(node, source, this->file);
        this->emitDiagnostic(diag);
    }
    void CSTVisitor::finalizeUnusedFrame(ts::Node node) {
        if (this->unusedStack.empty()) {
            return;
        }

        const UnusedFrame frame = this->unusedStack.back();
        this->unusedStack.pop_back();

        const size_t start = frame.pendingStart;
        const size_t total = this->unusedPendingNodes.size();

        uint32_t siblingSeq = 0u;
        if (!this->unusedStack.empty()) {
            siblingSeq = ++this->unusedStack.back().childCounter;
        }

        // Collapse entire unhandled subtree into its parent node
        if (frame.isUnused && !frame.hasUsedDescendant) {
            this->unusedPendingNodes.resize(start);
            if (!this->unusedStack.empty()) {
                this->unusedPendingNodes.push_back(PendingUnused{
                    .node = node,
                    .siblingSeq = siblingSeq,
                });
            } else {
                this->emitUnusedNodeDiagnostic(node, node);
            }
            return;
        }

        // Coalesce contiguous unused sibling runs in O(1) per sibling via `siblingSeq`
        if (!this->unusedStack.empty()) {
            this->unusedStack.back().hasUsedDescendant = true;
        }
        if (total > start) {
            std::span<const PendingUnused> pending(this->unusedPendingNodes.data() + start, total - start);
            ts::Node runStart = pending.front().node;
            ts::Node runEnd = runStart;
            uint32_t prevSeq = pending.front().siblingSeq;
            for (size_t i = 1; i < pending.size(); ++i) {
                const PendingUnused &curr = pending[i];
                if (curr.siblingSeq == prevSeq + 1u) {
                    runEnd = curr.node;
                } else {
                    this->emitUnusedNodeDiagnostic(runStart, runEnd);
                    runStart = curr.node;
                    runEnd = curr.node;
                }
                prevSeq = curr.siblingSeq;
            }
            this->emitUnusedNodeDiagnostic(runStart, runEnd);
            this->unusedPendingNodes.resize(start);
        }
    }
    void CSTVisitor::reportUnusedNode(ts::Node node, NodeState state) {
        if (state != NodeState::Valid || this->extraDepth > 0u) {
            return;
        }
        if (this->onDiagnostic == nullptr || !this->configs.reportUnusedCSTNodes) {
            return;
        }
        if (!this->unusedStack.empty()) {
            this->unusedStack.back().isUnused = true;
            return;
        }
        this->emitUnusedNodeDiagnostic(node, node);
    }

    // Node dispatch hooks
    ts::VisitAction CSTVisitor::onNode(ts::Node node) {
        const bool parentHandled = !this->traversalStack.empty() &&
            (this->traversalStack.back() == TraversalTag::ConsumerHandled ||
             this->traversalStack.back() == TraversalTag::ParentInherited);
        const bool consumerHandled = (this->consumer.onValidEnter != nullptr)
            ? this->consumer.onValidEnter(this->consumer.ctx, node, this->currentFieldId)
            : false;

        // Unnamed punctuation/keywords inherit handled status from their enclosing named CST rule
        const bool effectiveHandled = node.isNamed() ? consumerHandled : (consumerHandled || parentHandled);
        const TraversalTag tag = consumerHandled
            ? TraversalTag::ConsumerHandled
            : (effectiveHandled ? TraversalTag::ParentInherited : TraversalTag::ValidUnhandled);
        this->traversalStack.push_back(tag);

        if (!effectiveHandled) {
            this->reportUnusedNode(node, NodeState::Valid);
        }

        return ts::VisitAction::Continue;
    }
    ts::VisitAction CSTVisitor::onError(ts::Node node) {
        this->reportSyntaxError(node, NodeState::Error);
        return ts::VisitAction::SkipChildren;
    }
    ts::VisitAction CSTVisitor::onUnexpected(ts::Node node) {
        this->reportSyntaxError(node, NodeState::Unexpected);
        return ts::VisitAction::SkipChildren;
    }
    ts::VisitAction CSTVisitor::onMissing(ts::Node node) {
        this->reportSyntaxError(node, NodeState::Missing);
        return ts::VisitAction::SkipChildren;
    }

    void CSTVisitor::configure(CSTVisitorConfigs configurations, const File *targetFile,
        const ModifiableEvent *diagnosticCallback) noexcept {
        this->configs = configurations;
        this->file = targetFile;
        this->onDiagnostic = (diagnosticCallback != nullptr && *diagnosticCallback != nullptr)
            ? diagnosticCallback
            : nullptr;
    }

    void CSTVisitor::visit(const ts::Tree &tree) {
        this->visitImpl(tree.getRootNode(), ConsumerBridge{});
    }
    void CSTVisitor::visit(ts::Node root) {
        this->visitImpl(root, ConsumerBridge{});
    }

    ts::VisitAction CSTVisitor::onEnter(ts::Node node) {
        return this->onEnter(node, ts::FieldId::None);
    }
    ts::VisitAction CSTVisitor::onEnter(ts::Node node, ts::FieldId fieldId) {
        this->currentFieldId = fieldId;

        // Classify syntax errors BEFORE checking `isExtra()` (Tree-sitter marks `ERROR` nodes as extra)
        const NodeState state = classify(node);
        if (state != NodeState::Valid) {
            if (this->extraDepth == 0u && !this->unusedStack.empty()) {
                ++this->unusedStack.back().childCounter;
                this->unusedStack.back().hasUsedDescendant = true;
            }
            this->traversalStack.push_back(TraversalTag::ExtraOrError);
            switch (state) {
                case NodeState::Error:
                    return this->onError(node);
                case NodeState::Unexpected:
                    return this->onUnexpected(node);
                case NodeState::Missing:
                default:
                    return this->onMissing(node);
            }
        }

        // Skip valid comment / trivia subtrees without polluting `unusedStack`
        if (this->extraDepth > 0u) {
            this->traversalStack.push_back(TraversalTag::ExtraOrError);
            return ts::VisitAction::Continue;
        }
        if (node.isExtra()) {
            if (!node.hasError()) {
                this->traversalStack.push_back(TraversalTag::ExtraOrError);
                return ts::VisitAction::SkipChildren;
            }
            this->extraDepth = 1u;
            this->traversalStack.push_back(TraversalTag::ExtraScope);
            return ts::VisitAction::Continue;
        }

        const bool trackUnused = this->configs.reportUnusedCSTNodes && (this->onDiagnostic != nullptr);
        if (trackUnused) {
            this->unusedStack.push_back(UnusedFrame{
                .pendingStart = static_cast<uint32_t>(this->unusedPendingNodes.size()),
            });
        }

        const size_t depthBefore = this->traversalStack.size();
        const ts::VisitAction action = this->onNode(node);
        if (this->traversalStack.size() == depthBefore) {
            this->traversalStack.push_back(TraversalTag::ParentInherited);
        }
        return action;
    }
    void CSTVisitor::onLeave(ts::Node node) {
        if (this->traversalStack.empty()) {
            return;
        }

        const TraversalTag tag = this->traversalStack.back();
        this->traversalStack.pop_back();

        if (tag == TraversalTag::ExtraScope) {
            this->extraDepth = 0u;
            return;
        }
        if (tag == TraversalTag::ExtraOrError) {
            return;
        }

        if (tag == TraversalTag::ConsumerHandled && this->consumer.onValidLeave != nullptr) {
            this->consumer.onValidLeave(this->consumer.ctx, node);
        }
        if (!this->unusedStack.empty()) {
            this->finalizeUnusedFrame(node);
        }
    }

    // Static helpers
    NodeState CSTVisitor::classify(ts::Node node) noexcept {
        if (node.isNull()) {
            return NodeState::Error;
        }
        if (node.isError()) {
            const ts::Extent<uint32_t> bytes = node.getByteRange();
            return (node.getNumChildren() == 0u && bytes.end > bytes.start)
                ? NodeState::Unexpected
                : NodeState::Error;
        }
        if (node.isMissing()) {
            return NodeState::Missing;
        }
        return NodeState::Valid;
    }
    Diagnostic CSTVisitor::buildSyntaxDiagnostic(ts::Node node, std::string_view source, const File *originFile) {
        if (node.isNull()) {
            SET_CODE(diag, Error, 200008);
            diag.file = originFile;
            return diag;
        }

        switch (classify(node)) {
            case NodeState::Missing:
                return Internal::CSTDiagnostics::buildMissingDiagnostic(node, source, originFile);
            case NodeState::Unexpected:
                return Internal::CSTDiagnostics::buildUnexpectedDiagnostic(node, source, originFile);
            case NodeState::Error:
            default:
                return Internal::CSTDiagnostics::buildErrorDiagnostic(node, source, originFile);
        }
    }

    bool CSTVisitor::hasSyntaxErrors() const noexcept {
        return this->syntaxErrors;
    }
}
