/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#include "CSTVisitor.hpp"

#include "../internal/cstDiagnostics.hpp"

namespace Parser::Visitors {
    // Internal AST helpers
    void CSTVisitor::contaminateCurrentASTNode(ts::Node errorNode) {
        if (!(this->configs.buildAST) || this->astStack.empty()) {
            return;
        }

        ASTFrame &top = this->astStack.back();
        if (top.astNode.kind == ASTKind::Program || top.astNode.kind == ASTKind::Block) {
            this->astPendingChildren.push_back(PendingASTChild{
                .astNode = ASTNode{.kind = ASTKind::Dirty, .range = Internal::CSTDiagnostics::toRange(errorNode)},
                .cstFieldId = ts::FieldId::None,
            });
        } else {
            top.astNode.kind = ASTKind::Dirty;
        }
    }
    void CSTVisitor::finalizeASTFrame(ts::Node node, const NodeRule &rule) {
        if (!(this->configs.buildAST) || this->astStack.empty() || this->astStack.back().nodeId != node.getID()) {
            return;
        }

        // Grab stack
        ASTFrame frame = std::move(this->astStack.back());
        this->astStack.pop_back();

        // Save pending children in the tree
        const size_t start = frame.pendingStart;
        const size_t total = this->astPendingChildren.size();
        if (total > start) {
            const size_t count = total - start;
            std::span<PendingASTChild> children(this->astPendingChildren.data() + start, count);

            if (this->mapper != nullptr && rule.resolveChildRoles) {
                for (PendingASTChild &child : children) {
                    if (child.cstFieldId != ts::FieldId::None) {
                        child.astNode.role = this->mapper->lookupFieldRole(child.cstFieldId);
                    }
                }
            }

            // Save pending children in the tree
            frame.astNode.firstChild = static_cast<uint32_t>(this->ast->nodes.size());
            frame.astNode.childCount = static_cast<uint32_t>(count);
            for (const PendingASTChild &child : children) {
                this->ast->nodes.push_back(child.astNode);
            }

            this->astPendingChildren.resize(start);
        }

        if (rule.onLeaveHook != nullptr && frame.astNode.kind != ASTKind::Dirty) {
            rule.onLeaveHook(*this, node, frame.astNode);
        }

        // Save ASTNode in the tree
        if (!(this->astStack.empty())) {
            this->astPendingChildren.push_back(PendingASTChild{
                .astNode = frame.astNode,
                .cstFieldId = frame.cstFieldId,
            });
        } else if (frame.reservedSlot != ASTFrame::NO_SLOT) {
            this->ast->nodes[frame.reservedSlot] = frame.astNode;
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
        this->contaminateCurrentASTNode(node);
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

        const size_t start = frame.pendingStart;
        const size_t total = this->unusedPendingNodes.size();

        // Report entire group of nodes from (unused) stack
        if (frame.isUnused && !(frame.hasUsedDescendant)) {
            if (total > start) {
                this->unusedPendingNodes.resize(start);
            }
            if (!(this->unusedStack.empty())) {
                this->unusedPendingNodes.push_back(node);
            } else {
                this->emitUnusedNodeDiagnostic(node, node);
            }
            return;
        }

        // Report unused separate nodes from (unused) stack
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

            this->unusedPendingNodes.resize(start);
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
        // Ignore extra nodes
        if (node.isExtra() || this->extraRootId != ts::NodeID{}) {
            return ts::VisitAction::Continue;
        }

        // Report children of an unhandled parent
        if (!(node.isNamed())) {
            const bool parentHandled = !(this->handledStack.empty()) && this->handledStack.back();
            this->handledStack.push_back(parentHandled);
            if (!parentHandled) {
                this->reportUnusedNode(node, NodeState::Valid);
            }
            return ts::VisitAction::Continue;
        }

        const NodeRule &rule = (this->mapper != nullptr)
            ? this->mapper->lookupRule(node.getSymbol())
            : ASTMapper::unhandledRule();
        this->handledStack.push_back(rule.handled);

        // Report new unhandled node
        if (!rule.handled) {
            this->reportUnusedNode(node, NodeState::Valid);
            return ts::VisitAction::Continue;
        }

        // Build AST frame
        if (this->configs.buildAST && rule.astKind != ASTKind::None) {
            ASTFrame frame{
                .nodeId = node.getID(),
                .astNode = ASTNode{.kind = rule.astKind, .range = Internal::CSTDiagnostics::toRange(node)},
                .pendingStart = static_cast<uint32_t>(this->astPendingChildren.size()),
                .cstFieldId = this->currentFieldId,
            };

            // Add a new AST stack
            if (this->astStack.empty()) {
                frame.reservedSlot = static_cast<uint32_t>(this->ast->nodes.size());
                this->ast->nodes.emplace_back();
            }
            if (rule.onEnterHook != nullptr) {
                rule.onEnterHook(*this, node, frame.astNode);
            }
            this->astStack.push_back(frame);
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

    CSTVisitor::CSTVisitor(const ASTMapper *astMapper) noexcept
        : mapper(astMapper) {}

    void CSTVisitor::configure(CSTVisitorConfigs configurations, const File *targetFile,
        const ModifiableEvent *diagnosticCallback) noexcept {
        this->configs = configurations;
        this->file = targetFile;
        this->onDiagnostic = (diagnosticCallback != nullptr && *diagnosticCallback != nullptr)
            ? diagnosticCallback
            : nullptr;
    }

    void CSTVisitor::visit(const ts::Tree& tree) {
        this->visit(tree.getRootNode());
    }
    void CSTVisitor::visit(ts::Node root) {
        // Reset data
        this->unusedStack.clear();
        this->handledStack.clear();
        this->astStack.clear();
        this->unusedPendingNodes.clear();
        this->astPendingChildren.clear();
        this->extraRootId = ts::NodeID{};
        this->currentFieldId = ts::FieldId::None;
        this->ast = (this->configs.buildAST && !root.isNull()) ? std::make_unique<AST>() : nullptr;
        this->syntaxErrors = false;

        if (!root.isNull()) {
            ts::visitWithFields(root, *this);
        }

        // Clear tracking data
        this->unusedStack.clear();
        this->handledStack.clear();
        this->astStack.clear();
        this->unusedPendingNodes.clear();
        this->astPendingChildren.clear();
        this->extraRootId = ts::NodeID{};
        this->currentFieldId = ts::FieldId::None;
    }

    ts::VisitAction CSTVisitor::onEnter(ts::Node node) {
        return this->onEnter(node, ts::FieldId::None);
    }
    ts::VisitAction CSTVisitor::onEnter(ts::Node node, ts::FieldId fieldId) {
        this->currentFieldId = fieldId;

        // Track extra nodes (for exclusion from unused node tracking)
        const bool isExtra = node.isExtra();
        if (this->extraRootId == ts::NodeID{} && isExtra) {
            this->extraRootId = node.getID();
            if (!(this->unusedStack.empty())) {
                this->unusedStack.back().hasUsedDescendant = true;
            }
        }

        // Mark non-extra nodes as "unused" by default
        const bool trackUnused = this->configs.reportUnusedCSTNodes && this->onDiagnostic != nullptr && this->extraRootId == ts::NodeID{};
        if (trackUnused) {
            this->unusedStack.push_back(UnusedFrame{
                .nodeId = node.getID(),
                .pendingStart = static_cast<uint32_t>(this->unusedPendingNodes.size()),
            });
        }

        // Classify tree-sitter nodes
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
        // Handle AST node logic for valid nodes
        if (classify(node) == NodeState::Valid && !node.isExtra() && this->extraRootId == ts::NodeID{}) {
            if (node.isNamed()) {
                const NodeRule &rule = (this->mapper != nullptr)
                    ? this->mapper->lookupRule(node.getSymbol())
                    : ASTMapper::unhandledRule();
                if (rule.handled) {
                    if (rule.astKind != ASTKind::None) {
                        this->finalizeASTFrame(node, rule);
                    } else if (rule.onLeaveHook != nullptr && !node.hasError()) {
                        ASTNode dummy{};
                        rule.onLeaveHook(*this, node, dummy);
                    }
                }
            }
            if (!this->handledStack.empty()) {
                this->handledStack.pop_back();
            }
        }

        // Handle unused nodes
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

    // Queries & Output
    bool CSTVisitor::hasSyntaxErrors() const noexcept {
        return this->syntaxErrors;
    }
    std::unique_ptr<AST> CSTVisitor::takeAST() noexcept {
        return std::move(this->ast);
    }
}
