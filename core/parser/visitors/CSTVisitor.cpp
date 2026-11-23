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
    void CSTVisitor::reportUnusedNode(ts::Node node, NodeState state) {
        if (state != NodeState::Valid) {
            return;
        }

        if (this->onDiagnostic == nullptr || !(this->configs.reportUnusedCSTNodes)) {
            return;
        }

        Diagnostic diag = Internal::CSTDiagnostics::buildDiagnostic(node, this->file) ;
        USE_CODE(diag, Hint, 200009);
        this->emitDiagnostic(diag);
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
