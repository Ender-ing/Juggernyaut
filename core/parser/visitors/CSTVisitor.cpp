/**
 * @brief 
 * Tree-sitter CST walker, syntax error reporter, and CSTConsumer driver
**/

#include "CSTVisitor.hpp"

#include "../internal/cstDiagnostics.hpp"

namespace Parser::Visitors {
    void CSTVisitor::resetState() noexcept {
        this->unusedStack.clear();
        this->unusedPendingNodes.clear();
        this->handledStack.clear();
        this->extraRootId = ts::NodeID{};
        this->currentFieldId = ts::FieldId::None;
        this->consumer = ConsumerBridge{};
    }
    void CSTVisitor::visitImpl(ts::Node root, ConsumerBridge boundConsumer) {
        struct VisitScopeGuard {
            CSTVisitor *self;
            ~VisitScopeGuard() noexcept {
                self->resetState();
            }
        } guard{this};

        this->resetState();
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
        if (this->onDiagnostic == nullptr || !(this->configs.reportUnusedCSTNodes)) {
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

        // Collapse entire unhandled subtree into its parent node
        if (frame.isUnused && !(frame.hasUsedDescendant)) {
            if (total > start) {
                this->unusedPendingNodes.erase(
                    this->unusedPendingNodes.begin() + static_cast<std::ptrdiff_t>(start),
                    this->unusedPendingNodes.end()
                );
            }
            if (!(this->unusedStack.empty())) {
                this->unusedPendingNodes.push_back(node);
            } else {
                this->emitUnusedNodeDiagnostic(node, node);
            }
            return;
        }

        // Coalesce contiguous unused sibling runs
        if (!(this->unusedStack.empty())) {
            this->unusedStack.back().hasUsedDescendant = true;
        }
        if (total > start) {
            std::span<const ts::Node> pending(this->unusedPendingNodes.data() + start, total - start);
            ts::Node runStart = pending.front();
            ts::Node runEnd = runStart;
            for (size_t i = 1; i < pending.size(); ++i) {
                const ts::Node curr = pending[i];
                const std::optional<ts::Node> next = runEnd.getNextSibling();
                if (next.has_value() && next->impl.id == curr.impl.id) {
                    runEnd = curr;
                } else {
                    this->emitUnusedNodeDiagnostic(runStart, runEnd);
                    runStart = curr;
                    runEnd = curr;
                }
            }
            this->emitUnusedNodeDiagnostic(runStart, runEnd);

            this->unusedPendingNodes.erase(
                this->unusedPendingNodes.begin() + static_cast<std::ptrdiff_t>(start),
                this->unusedPendingNodes.end()
            );
        }
    }
    void CSTVisitor::reportUnusedNode(ts::Node node, NodeState state) {
        if (state != NodeState::Valid || node.isExtra() || this->extraRootId != ts::NodeID{}) {
            return;
        }
        if (this->onDiagnostic == nullptr || !(this->configs.reportUnusedCSTNodes)) {
            return;
        }
        if (!this->unusedStack.empty() && this->unusedStack.back().nodeId == node.getID()) {
            this->unusedStack.back().isUnused = true;
            return;
        }
        this->emitUnusedNodeDiagnostic(node, node);
    }

    ts::VisitAction CSTVisitor::onNode(ts::Node node) {
        // Ignore extra nodes (comments / trivia)
        if (node.isExtra() || this->extraRootId != ts::NodeID{}) {
            return ts::VisitAction::Continue;
        }

        const bool parentHandled = !(this->handledStack.empty()) && (this->handledStack.back() != 0u);
        const bool consumerHandled = (this->consumer.onValidEnter != nullptr)
            ? this->consumer.onValidEnter(this->consumer.ctx, node, this->currentFieldId)
            : false;

        // Unnamed punctuation/keywords inherit handled status from their enclosing named CST rule
        const bool effectiveHandled = node.isNamed() ? consumerHandled : (consumerHandled || parentHandled);
        this->handledStack.push_back(effectiveHandled ? uint8_t{1} : uint8_t{0});

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

        // Track extra subtrees (for exclusion from unused-node tracking)
        const bool isExtra = node.isExtra();
        if (this->extraRootId == ts::NodeID{} && isExtra) {
            this->extraRootId = node.getID();
            if (!(this->unusedStack.empty())) {
                this->unusedStack.back().hasUsedDescendant = true;
            }
        }

        const bool trackUnused = this->configs.reportUnusedCSTNodes &&
            this->onDiagnostic != nullptr &&
            this->extraRootId == ts::NodeID{};
        if (trackUnused) {
            this->unusedStack.push_back(UnusedFrame{
                .nodeId = node.getID(),
                .pendingStart = static_cast<uint32_t>(this->unusedPendingNodes.size()),
            });
        }

        switch (classify(node)) {
            case NodeState::Valid:
                return this->onNode(node);
            case NodeState::Error:
                return this->onError(node);
            case NodeState::Unexpected:
                return this->onUnexpected(node);
            case NodeState::Missing:
                return this->onMissing(node);
            default:
                return ts::VisitAction::Continue;
        }
    }
    void CSTVisitor::onLeave(ts::Node node) {
        if (classify(node) == NodeState::Valid && !node.isExtra() && this->extraRootId == ts::NodeID{}) {
            if (this->consumer.onValidLeave != nullptr) {
                this->consumer.onValidLeave(this->consumer.ctx, node);
            }
            if (!this->handledStack.empty()) {
                this->handledStack.pop_back();
            }
        }

        if (!(this->unusedStack.empty()) && this->unusedStack.back().nodeId == node.getID()) {
            this->finalizeUnusedFrame(node);
        }
        if (node.getID() == this->extraRootId) {
            this->extraRootId = ts::NodeID{};
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
