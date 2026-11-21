/**
 * @brief 
 * A basic tree-sitter CST visitors
**/

#include "CSTVisitor.hpp"

namespace Parser::Visitors {
    ts::VisitAction CSTVisitor::onNode(ts::Node) {
        return ts::VisitAction::Continue;
    }
    ts::VisitAction CSTVisitor::onError(ts::Node) {
        return ts::VisitAction::SkipChildren;
    }
    ts::VisitAction CSTVisitor::onUnexpected(ts::Node) {
        return ts::VisitAction::SkipChildren;
    }
    ts::VisitAction CSTVisitor::onMissing(ts::Node) {
        return ts::VisitAction::SkipChildren;
    }

    void CSTVisitor::visit(const ts::Tree& tree) {
        this->visit(tree.getRootNode());
    }
    void CSTVisitor::visit(ts::Node root) {
        if (!root.isNull()) {
            ts::visit(root, *this);
        }
    }

    ts::VisitAction CSTVisitor::onEnter(ts::Node node) {
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
                break;
        }
        return ts::VisitAction::Continue;
    }
    void CSTVisitor::onLeave(ts::Node) {}

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
}
