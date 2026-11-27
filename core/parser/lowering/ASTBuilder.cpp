/**
 * @brief
 * Stateful per-parse CSTConsumer that assembles flat ASTs using a LoweringTable
**/

#include "ASTBuilder.hpp"

// C++ headers
#include <algorithm>

namespace Parser::Lowering {
    namespace Helper {
        constexpr size_t MAX_RETAINED_STACK_CAPACITY = 4096u;
        constexpr size_t MAX_RETAINED_PENDING_CAPACITY = 65536u;
        constexpr size_t MAX_INITIAL_AST_RESERVE = 1048576u; // 1M nodes (~32 MB cap on initial reserve)

        template <typename T>
        void resetBuffer(std::vector<T> &vec, bool trimExcess, size_t maxRetainedCap) noexcept {
            vec.clear();
            if (trimExcess && vec.capacity() > maxRetainedCap) {
                std::vector<T>{}.swap(vec);
            }
        }
    }

    void ASTBuilder::clearScratch(bool trimExcessCapacity) noexcept {
        Helper::resetBuffer(this->cstStack, trimExcessCapacity, Helper::MAX_RETAINED_STACK_CAPACITY);
        Helper::resetBuffer(this->astStack, trimExcessCapacity, Helper::MAX_RETAINED_STACK_CAPACITY);
        Helper::resetBuffer(this->astPendingChildren, trimExcessCapacity, Helper::MAX_RETAINED_PENDING_CAPACITY);
        Helper::resetBuffer(this->astPendingFields, trimExcessCapacity, Helper::MAX_RETAINED_PENDING_CAPACITY);
        Helper::resetBuffer(this->forwardedFieldStack, trimExcessCapacity, Helper::MAX_RETAINED_STACK_CAPACITY);
    }
    void ASTBuilder::finalizeFrame(ts::Node node, const SymbolRule &rule) {
        if (this->ast == nullptr || this->astStack.empty() ||
            this->astStack.back().nodeId != node.getID()) {
            return;
        }

        ASTFrame frame = this->astStack.back();
        this->astStack.pop_back();

        const size_t start = frame.pendingStart;
        const size_t total = this->astPendingChildren.size();

        if (total > start) {
            const size_t count = total - start;
            std::span<ASTNode> children(this->astPendingChildren.data() + start, count);

            if (this->table != nullptr && rule.resolveChildRoles) {
                std::span<const ts::FieldId> fields(this->astPendingFields.data() + start, count);
                const ASTKind roleOwner = (frame.astNode.kind != ASTKind::Dirty) ? frame.astNode.kind : rule.astKind;
                for (size_t i = 0; i < count; ++i) {
                    if (fields[i] != ts::FieldId::None) {
                        children[i].role = this->table->lookupRole(roleOwner, fields[i]);
                    }
                }
            }

            frame.astNode.firstChild = static_cast<uint32_t>(this->ast->nodes.size());
            frame.astNode.childCount = static_cast<uint32_t>(count);
            this->ast->nodes.insert(this->ast->nodes.end(), children.begin(), children.end());

            this->astPendingChildren.resize(start);
            this->astPendingFields.resize(start);
        }

        if (rule.onLeave != nullptr && (!frame.astNode.hasError() || rule.runHooksOnError)) {
            rule.onLeave(*this, node, frame.astNode);
        }

        if (!this->astStack.empty()) {
            const uint32_t stagedIdx = static_cast<uint32_t>(this->astPendingChildren.size());
            this->astPendingChildren.push_back(frame.astNode);
            this->astPendingFields.push_back(frame.cstFieldId);
            if (rule.attachToNext) {
                ASTFrame &parent = this->astStack.back();
                if (parent.prefixStart == ASTFrame::NO_SLOT) {
                    parent.prefixStart = stagedIdx;
                }
            }
        } else if (frame.reservedSlot != ASTFrame::NO_SLOT) {
            this->ast->nodes[frame.reservedSlot] = frame.astNode;
        }
    }

    ASTBuilder::ASTBuilder(const LoweringTable *loweringTable) noexcept
        : table(loweringTable) {}

    void ASTBuilder::setTable(const LoweringTable *loweringTable) noexcept {
        this->table = loweringTable;
    }
    const LoweringTable* ASTBuilder::getTable() const noexcept {
        return this->table;
    }

    void ASTBuilder::reset(const File *targetFile, std::vector<ImportRequest> *importSink,
        const LoweringTable *overrideTable, void *perParseUserContext) {
        this->clearScratch(false);
        if (overrideTable != nullptr) {
            this->table = overrideTable;
        }
        this->file = targetFile;
        this->imports = importSink;
        this->userContext = perParseUserContext;
        this->ast = std::make_unique<AST>();

        // ~1 AST node per 8 source bytes avoids repeated multi-megabyte vector reallocations
        if (targetFile != nullptr) {
            const size_t estimatedNodes = std::min<size_t>(
                targetFile->content.byteLength() / 8u,
                Helper::MAX_INITIAL_AST_RESERVE
            );
            if (estimatedNodes > 0u) {
                this->ast->nodes.reserve(estimatedNodes);
            }
        }
    }

    bool ASTBuilder::onValidEnter(ts::Node node, ts::FieldId fieldId) {
        if (this->table == nullptr) {
            return false;
        }

        const ts::Symbol sym = node.getSymbol();
        const ts::Symbol cstParent = !this->cstStack.empty()
            ? this->cstStack.back().symbol
            : LoweringTable::ANY_CST_SYMBOL;
        const ASTKind astParent = !this->astStack.empty()
            ? this->astStack.back().astNode.kind
            : ASTKind::None;

        const SymbolRule &rule = this->table->lookupSymbol(sym, cstParent, astParent);
        switch (rule.op) {
            case LoweringOp::Unhandled:
                return false;

            case LoweringOp::Ignore: {
                this->cstStack.push_back(ActiveCSTEntry{.symbol = sym, .rule = &rule});
                if (rule.onEnter != nullptr) {
                    ASTNode dummy{};
                    rule.onEnter(*this, node, dummy);
                }
                return true;
            }

            case LoweringOp::MutateParent: {
                this->cstStack.push_back(ActiveCSTEntry{.symbol = sym, .rule = &rule});
                if (!this->astStack.empty()) {
                    ASTFrame &parent = this->astStack.back();
                    if (rule.targetParent == ASTKind::None || parent.astNode.kind == rule.targetParent) {
                        if (rule.astKind != ASTKind::None && parent.astNode.kind != ASTKind::Dirty) {
                            parent.astNode.kind = rule.astKind;
                        }
                        if (rule.subKind != 0u && (rule.clobberSubKind || parent.astNode.subKind == 0u)) {
                            parent.astNode.subKind = rule.subKind;
                        }
                        parent.astNode.flags = static_cast<uint16_t>(parent.astNode.flags | rule.flags);
                        if (rule.onEnter != nullptr) {
                            rule.onEnter(*this, node, parent.astNode);
                        }
                    }
                }
                return true;
            }

            case LoweringOp::MutateNext: {
                this->cstStack.push_back(ActiveCSTEntry{.symbol = sym, .rule = &rule});
                if (!this->astStack.empty()) {
                    ASTFrame &scope = this->astStack.back();
                    if (rule.targetParent == ASTKind::None || scope.astNode.kind == rule.targetParent) {
                        if (rule.subKind != 0u && (rule.clobberSubKind || scope.pendingNextSubKind == 0u)) {
                            scope.pendingNextSubKind = rule.subKind;
                        }
                        scope.pendingNextFlags = static_cast<uint16_t>(scope.pendingNextFlags | rule.flags);
                        if (rule.onEnter != nullptr) {
                            ASTNode dummy{};
                            rule.onEnter(*this, node, dummy);
                        }
                    }
                }
                return true;
            }

            case LoweringOp::PassThrough: {
                this->cstStack.push_back(ActiveCSTEntry{.symbol = sym, .rule = &rule});
                if (!this->astStack.empty()) {
                    ASTFrame &parent = this->astStack.back();
                    const ts::FieldId prev = parent.activeForwardedField;
                    const ts::FieldId next = rule.forwardField
                        ? ((fieldId != ts::FieldId::None) ? fieldId : prev)
                        : ts::FieldId::None;
                    if (next != prev) {
                        this->forwardedFieldStack.push_back(ForwardedFieldFrame{
                            .nodeId = node.getID(),
                            .prevField = prev,
                        });
                        parent.activeForwardedField = next;
                    }
                }
                if (rule.onEnter != nullptr) {
                    ASTNode dummy{};
                    rule.onEnter(*this, node, dummy);
                }
                return true;
            }

            case LoweringOp::EmitNode: {
                this->cstStack.push_back(ActiveCSTEntry{.symbol = sym, .rule = &rule});
                if (this->ast == nullptr) {
                    this->ast = std::make_unique<AST>();
                }

                ts::FieldId effectiveField = fieldId;
                uint32_t startOffset = static_cast<uint32_t>(this->astPendingChildren.size());
                uint16_t initialSubKind = rule.subKind;
                uint16_t initialFlags = rule.flags;

                if (!this->astStack.empty()) {
                    ASTFrame &parent = this->astStack.back();
                    if (effectiveField == ts::FieldId::None) {
                        effectiveField = parent.activeForwardedField;
                    }
                    // Claim staged sibling-prefix modifiers & prefix nodes (e.g. `@directive`, `export`)
                    if (!rule.attachToNext) {
                        if (parent.pendingNextSubKind != 0u && initialSubKind == 0u) {
                            initialSubKind = parent.pendingNextSubKind;
                        }
                        initialFlags = static_cast<uint16_t>(initialFlags | parent.pendingNextFlags);
                        parent.pendingNextSubKind = 0u;
                        parent.pendingNextFlags = 0u;

                        if (parent.prefixStart != ASTFrame::NO_SLOT) {
                            startOffset = parent.prefixStart;
                            parent.prefixStart = ASTFrame::NO_SLOT;
                        }
                    }
                }

                const ts::Extent<uint32_t> bytes = node.getByteRange();
                Data::Parser::LexemeId lexId = Data::Parser::INVALID_LEXEME_ID;
                if (rule.internText && this->file != nullptr) {
                    lexId = this->ast->internLexeme(this->sourceText(node));
                }

                ASTFrame frame{
                    .nodeId = node.getID(),
                    .astNode = ASTNode{
                        .kind = rule.astKind,
                        .subKind = initialSubKind,
                        .flags = initialFlags,
                        .span = Data::Parser::ByteSpan{.start = bytes.start, .end = bytes.end},
                        .lexemeId = lexId,
                    },
                    .pendingStart = startOffset,
                    .cstFieldId = effectiveField,
                };

                if (this->astStack.empty()) {
                    frame.reservedSlot = static_cast<uint32_t>(this->ast->nodes.size());
                    this->ast->nodes.emplace_back();
                }
                if (rule.onEnter != nullptr) {
                    rule.onEnter(*this, node, frame.astNode);
                }
                this->astStack.push_back(frame);
                return true;
            }

            default:
                return false;
        }
    }
    void ASTBuilder::onValidLeave(ts::Node node) {
        if (this->table == nullptr || this->cstStack.empty()) {
            return;
        }

        const ActiveCSTEntry active = this->cstStack.back();
        this->cstStack.pop_back();

        const SymbolRule &rule = *active.rule;
        if (rule.op == LoweringOp::PassThrough &&
            !this->forwardedFieldStack.empty() &&
            this->forwardedFieldStack.back().nodeId == node.getID()) {
            const ts::FieldId restored = this->forwardedFieldStack.back().prevField;
            this->forwardedFieldStack.pop_back();
            if (!this->astStack.empty()) {
                this->astStack.back().activeForwardedField = restored;
            }
        }

        if (rule.op == LoweringOp::EmitNode) {
            this->finalizeFrame(node, rule);
        } else if (rule.onLeave != nullptr && (!node.hasError() || rule.runHooksOnError)) {
            if (rule.op == LoweringOp::MutateParent) {
                if (!this->astStack.empty()) {
                    ASTFrame &parent = this->astStack.back();
                    if (rule.targetParent == ASTKind::None || parent.astNode.kind == rule.targetParent) {
                        rule.onLeave(*this, node, parent.astNode);
                    }
                }
            } else {
                ASTNode dummy{};
                rule.onLeave(*this, node, dummy);
            }
        }
    }
    void ASTBuilder::onSyntaxError(ts::Node errorNode) {
        if (this->astStack.empty()) {
            return;
        }

        ASTFrame &top = this->astStack.back();
        // Preserve `top.astNode.kind` so valid sibling/body subtrees stay analyzable in IDE/semantic passes
        top.astNode.flags = static_cast<uint16_t>(top.astNode.flags | Data::Parser::ASTFlags::HasError);
        top.prefixStart = ASTFrame::NO_SLOT;
        top.pendingNextSubKind = 0u;
        top.pendingNextFlags = 0u;

        const ts::Extent<uint32_t> bytes = errorNode.getByteRange();
        this->astPendingChildren.push_back(ASTNode{
            .kind = ASTKind::Dirty,
            .flags = Data::Parser::ASTFlags::HasError,
            .span = Data::Parser::ByteSpan{.start = bytes.start, .end = bytes.end},
        });
        this->astPendingFields.push_back(top.activeForwardedField);
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
    std::span<const ASTNode> ASTBuilder::childrenOf(const ASTNode &parent) const noexcept {
        if (this->ast == nullptr || parent.childCount == 0u ||
            (static_cast<size_t>(parent.firstChild) + parent.childCount) > this->ast->nodes.size()) {
            return {};
        }
        return std::span<const ASTNode>(this->ast->nodes.data() + parent.firstChild, parent.childCount);
    }
    ASTNode* ASTBuilder::mutableChildAt(const ASTNode &parent, uint32_t index) noexcept {
        if (this->ast == nullptr || index >= parent.childCount) {
            return nullptr;
        }
        const size_t absIndex = static_cast<size_t>(parent.firstChild) + index;
        return absIndex < this->ast->nodes.size() ? &this->ast->nodes[absIndex] : nullptr;
    }
    void ASTBuilder::pushImport(ImportRequest &&request) {
        if (this->imports != nullptr) {
            this->imports->push_back(std::move(request));
        }
    }

    // Output extraction
    std::unique_ptr<AST> ASTBuilder::takeAST() noexcept {
        this->clearScratch(true);
        this->file = nullptr;
        this->imports = nullptr;
        this->userContext = nullptr;
        return std::move(this->ast);
    }
}
