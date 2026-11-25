/**
 * @brief
 * Handle tree-sitter inclusion
**/

#pragma once

#ifdef _MSC_VER
    #pragma warning(push, 0) // Sets warning level to 0 (off) for everything below
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC system_header // Tells GCC/Clang to treat everything below as a system header
#endif

// Include tree-sitter libs
#include <cpp-tree-sitter.h>

// Extend the 'ts' namespace
namespace ts {
    // Direct cursor
    struct TraversalCursor {
        TSTreeCursor raw{};
        TraversalCursor() noexcept = default;
        ~TraversalCursor() {
            ts_tree_cursor_delete(&this->raw);
        }
        TraversalCursor(const TraversalCursor&) = delete;
        TraversalCursor& operator=(const TraversalCursor&) = delete;
        TraversalCursor(TraversalCursor&& other) noexcept : raw(other.raw) {
            other.raw = TSTreeCursor{};
        }
        TraversalCursor& operator=(TraversalCursor&& other) noexcept {
            if (this != &other) {
                ts_tree_cursor_delete(&this->raw);
                this->raw = other.raw;
                other.raw = TSTreeCursor{};
            }
            return *this;
        }
        void reset(ts::Node root) noexcept {
            ts_tree_cursor_reset(&this->raw, root.impl);
        }
        void clearTree() noexcept {
            this->raw.tree = nullptr;
        }
        [[nodiscard]] ts::Node currentNode() const noexcept {
            return ts::Node{ts_tree_cursor_current_node(&this->raw)};
        }
        [[nodiscard]] ts::FieldId currentFieldId() const noexcept {
            return ts::FieldId{ts_tree_cursor_current_field_id(&this->raw)};
        }
        [[nodiscard]] bool gotoFirstChild() noexcept {
            return ts_tree_cursor_goto_first_child(&this->raw);
        }
        [[nodiscard]] bool gotoNextSibling() noexcept {
            return ts_tree_cursor_goto_next_sibling(&this->raw);
        }
        [[nodiscard]] bool gotoParent() noexcept {
            return ts_tree_cursor_goto_parent(&this->raw);
        }
    };

    // Visitor (with exposed field IDs)
    template <typename V>
    concept FieldTreeVisitor = requires(V visitor, Node node, FieldId fieldId) {
        { visitor.onEnter(node, fieldId) } -> std::same_as<VisitAction>;
    };
    template <FieldTreeVisitor V>
    void visitWithFields(Node root, V&& visitor) {
        TraversalCursor cursor;
        cursor.reset(root);
        while (true) {
            const VisitAction action = visitor.onEnter(cursor.currentNode(), cursor.currentFieldId()); // This is needed!
            if (action == VisitAction::Stop) {
                return;
            }
            if (action != VisitAction::SkipChildren && cursor.gotoFirstChild()) {
                continue;
            }
            while (true) {
                if constexpr (requires { visitor.onLeave(cursor.currentNode()); }) {
                    visitor.onLeave(cursor.currentNode());
                }
                if (cursor.gotoNextSibling()) {
                    break;
                }
                if (!cursor.gotoParent()) {
                    return;
                }
            }
        }
    }
}

// Restore state
#ifdef _MSC_VER
    #pragma warning(pop)
#elif defined(__GNUC__) || defined(__clang__)
    #pragma GCC diagnostic pop
#endif
