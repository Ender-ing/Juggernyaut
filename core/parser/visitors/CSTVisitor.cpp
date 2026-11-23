/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#include "CSTVisitor.hpp"

#include "../internal/cstDiagnostics.hpp"

namespace Parser::Visitors {
    // Diagnostics
    void CSTVisitor::emitDiagnostic(Diagnostic &diag) {
        diag.file = this->file;
        if (this->onDiagnostic != nullptr) {
            this->onDiagnostic(diag);
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
        UnusedFrame frame = std::move(this->unusedStack.back());
        this->unusedStack.pop_back();
        if (frame.isUnused && !(frame.hasUsedDescendant)) {
            if (!(this->unusedStack.empty())) {
                this->unusedStack.back().pending.push_back(node);
            } else {
                this->emitUnusedNodeDiagnostic(node, node);
            }
            return;
        }
        if (!(this->unusedStack.empty())) {
            this->unusedStack.back().hasUsedDescendant = true;
        }
        if (!frame.pending.empty()) {
            ts::Node runStart = frame.pending.front();
            ts::Node runEnd = runStart;
            for (size_t i = 1; i < frame.pending.size(); ++i) {
                ts::Node curr = frame.pending[i];
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
        }
    }
    void CSTVisitor::reportUnusedNode(ts::Node node, NodeState state) {
        if (state != NodeState::Valid) {
            return;
        }
        if (this->onDiagnostic == nullptr || !(this->configs.reportUnusedCSTNodes)) {
            return;
        }
        if (!this->unusedStack.empty() && this->unusedStack.back().nodeId == node.impl.id) {
            this->unusedStack.back().isUnused = true;
            return;
        }
        this->emitUnusedNodeDiagnostic(node, node);
    }

    ts::VisitAction CSTVisitor::onNode(ts::Node node) {
        this->reportUnusedNode(node, NodeState::Valid);
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

    void CSTVisitor::visit(const ts::Tree& tree) {
        this->visit(tree.getRootNode());
    }
    void CSTVisitor::visit(ts::Node root) {
        this->unusedStack.clear();
        if (!root.isNull()) {
            ts::visit(root, *this);
        }
        this->unusedStack.clear();
    }

    ts::VisitAction CSTVisitor::onEnter(ts::Node node) {
        const bool trackUnused = this->configs.reportUnusedCSTNodes && this->onDiagnostic != nullptr;
        if (trackUnused) {
            this->unusedStack.push_back(UnusedFrame{.nodeId = node.impl.id});
        }

        ts::VisitAction action = ts::VisitAction::Continue;
        switch (classify(node)) {
            case NodeState::Valid:
                action = this->onNode(node);
                break;
            case NodeState::Error:
                action = this->onError(node);
                break;
            case NodeState::Unexpected:
                action = this->onUnexpected(node);
                break;
            case NodeState::Missing:
                action = this->onMissing(node);
                break;
            default:
                break;
        }

        if (trackUnused && action == ts::VisitAction::SkipChildren) {
            this->finalizeUnusedFrame(node);
        }
        return action;
    }
    void CSTVisitor::onLeave(ts::Node node) {
        if (!(this->unusedStack.empty()) && this->unusedStack.back().nodeId == node.impl.id) {
            this->finalizeUnusedFrame(node);
        }
    }

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
}
