/**
 * @brief
 * Stateful per-parse CSTConsumer that assembles flat ASTs using a LoweringTable
**/

#include "ASTBuilder.hpp"

#include <algorithm>
#include "../internal/cstDiagnostics.hpp"

namespace Parser::Lowering {
    ASTBuilder::ASTBuilder(const LoweringTable *loweringTable) noexcept
        : table(loweringTable) {}

    void ASTBuilder::clearScratch() noexcept {
        this->astStack.clear();
        this->astPendingChildren.clear();
        this->forwardedFieldStack.clear();
    }
    void ASTBuilder::finalizeFrame(ts::Node node, const SymbolRule &rule) {
        // Ignore non-matching stacks
        if (this->ast == nullptr || this->astStack.empty() ||
            this->astStack.back().nodeId != node.getID()) {
            return;
        }

        // Grab frame
        ASTFrame frame = this->astStack.back();
        this->astStack.pop_back();

        const size_t start = frame.pendingStart;
        const size_t total = this->astPendingChildren.size();

        if (total > start) {
            const size_t count = total - start;
            std::span<PendingASTChild> children(this->astPendingChildren.data() + start, count);

            // Process the children of the current node
            if (this->table != nullptr && rule.resolveChildRoles) {
                const ASTKind roleOwner = (frame.astNode.kind != ASTKind::Dirty) ? frame.astNode.kind : rule.astKind;
                for (PendingASTChild &child : children) {
                    if (child.cstFieldId != ts::FieldId::None) {
                        // Process children's role
                        child.astNode.role = this->table->lookupRole(roleOwner, child.cstFieldId);
                    }
                }
            }

            // Save children's AST location
            frame.astNode.firstChild = static_cast<uint32_t>(this->ast->nodes.size());
            frame.astNode.childCount = static_cast<uint32_t>(count);
            for (const PendingASTChild &child : children) {
                this->ast->nodes.push_back(child.astNode);
            }

            // Delete processed up children
            this->astPendingChildren.resize(start);
        }

        // Trigger 'leave' event for valid nodes
        if (rule.onLeave != nullptr && frame.astNode.kind != ASTKind::Dirty) {
            rule.onLeave(*this, node, frame.astNode);
        }

        // Queue up frame's node
        if (!(this->astStack.empty())) {
            this->astPendingChildren.push_back(PendingASTChild{
                .astNode = frame.astNode,
                .cstFieldId = frame.cstFieldId,
            });
        } else if (frame.reservedSlot != ASTFrame::NO_SLOT) {
            this->ast->nodes[frame.reservedSlot] = frame.astNode;
        }
    }

    void ASTBuilder::reset(const File *targetFile, std::vector<ImportRequest> *importSink) {
        this->clearScratch();
        this->file = targetFile;
        this->imports = importSink;
        this->ast = std::make_unique<AST>();

        // Prevent frequest vector reallocations
        if (targetFile != nullptr) {
            const size_t estimatedNodes = std::min<size_t>(targetFile->content.byteLength() / 32u, 65536u); // ~1.75 MB cap
            if (estimatedNodes > 0u) {
                this->ast->nodes.reserve(estimatedNodes);
            }
        }
    }

    // Context helpers exposed to `SymbolRule::Hook`
    const File* ASTBuilder::currentFile() const noexcept {
        return this->file;
    }
    AST* ASTBuilder::currentAST() noexcept {
        return this->ast.get();
    }
    std::string_view ASTBuilder::sourceText(ts::Node node) const noexcept {
        if (this->file == nullptr || node.isNull()) {
            return {};
        }

        const std::string_view src = this->file->content.view();
        const ts::Extent<uint32_t> bytes = node.getByteRange();
        if (bytes.start >= src.size() || bytes.end <= bytes.start) {
            return {};
        }

        const size_t length = std::min<size_t>(bytes.end - bytes.start, src.size() - bytes.start);
        return src.substr(bytes.start, length);
    }
    std::span<ASTNode> ASTBuilder::childrenOf(const ASTNode &parent) noexcept {
        if (this->ast == nullptr || parent.childCount == 0u ||
            (static_cast<size_t>(parent.firstChild) + parent.childCount) > this->ast->nodes.size()) {
            return {};
        }
        return std::span<ASTNode>(this->ast->nodes.data() + parent.firstChild, parent.childCount);
    }
    void ASTBuilder::pushImport(ImportRequest &&request) {
        if (this->imports != nullptr) {
            this->imports->push_back(std::move(request));
        }
    }

    // Output extraction
    std::unique_ptr<AST> ASTBuilder::takeAST() noexcept {
        this->clearScratch();
        this->file = nullptr;
        this->imports = nullptr;
        return std::move(this->ast);
    }
}
