/**
 * @brief
 * Internal Tree-sitter CST syntax diagnostic builders
**/

#include "cstDiagnostics.hpp"

// C++ headers
#include <algorithm>
#include <format>

namespace Parser::Internal::CSTDiagnostics {
    // (AI-generated)
    namespace Helper {
        constexpr size_t MAX_EXPECTED = 6;
        constexpr size_t MAX_SNIPPET = 24;

        constexpr Position toPosition(ts::Point pt) noexcept {
            return Position{.line = pt.row, .character = pt.column};
        }

        Range toRange(ts::Node node) noexcept {
            const ts::Extent<ts::Point> pts = node.getPointRange();
            return Range{.start = toPosition(pts.start), .end = toPosition(pts.end)};
        }

        std::optional<ts::Node> findPrevSibling(ts::Node node) noexcept {
            std::optional<ts::Node> prev = node.getPreviousSibling();
            while (prev.has_value() && prev->isExtra()) {
                prev = prev->getPreviousSibling();
            }
            return prev;
        }

        ts::Node firstLeaf(ts::Node node) noexcept {
            ts::Cursor c = node.getCursor();
            while (c.gotoFirstChild()) {
                while (c.getCurrentNode().isExtra() && c.gotoNextSibling()) {}
                if (c.getCurrentNode().isExtra()) {
                    (void)c.gotoParent();
                    break;
                }
            }
            return c.getCurrentNode();
        }

        ts::Node lastLeaf(ts::Node node) noexcept {
            ts::Cursor c = node.getCursor();
            while (c.gotoLastChild()) {
                while (c.getCurrentNode().isExtra() && c.gotoPreviousSibling()) {}
                if (c.getCurrentNode().isExtra()) {
                    (void)c.gotoParent();
                    break;
                }
            }
            return c.getCurrentNode();
        }

        std::string formatSymbol(std::string_view rawName, bool isNamed) {
            if (!isNamed) {
                return std::format("'{}'", rawName);
            }
            std::string out(rawName);
            std::ranges::replace(out, '_', ' ');
            return out;
        }

        std::string extractSnippet(ts::Node leaf, std::string_view source) {
            if (source.empty()) {
                return {};
            }
            std::string_view text = leaf.getSourceRange(source);
            text = text.substr(0, text.find_first_of("\r\n"));
            if (text.size() > MAX_SNIPPET) {
                return std::format("{}...", text.substr(0, MAX_SNIPPET));
            }
            return std::string(text);
        }

        std::string expectedFromState(ts::Language lang, TSStateId state) {
            if (lang.impl == nullptr || state == 0u) {
                return {};
            }

            TSLookaheadIterator* iter = ts_lookahead_iterator_new(lang.impl, state);
            if (iter == nullptr) {
                return {};
            }

            std::string out;
            size_t count = 0;
            while (ts_lookahead_iterator_next(iter)) {
                const TSSymbol raw = ts_lookahead_iterator_current_symbol(iter);
                if (raw == 0u || raw == static_cast<TSSymbol>(-1)) {
                    continue;
                }

                const TSSymbolType kind = ts_language_symbol_type(lang.impl, raw);
                if (kind != TSSymbolTypeRegular && kind != TSSymbolTypeAnonymous) {
                    continue;
                }

                const std::string_view name = ts_lookahead_iterator_current_symbol_name(iter);
                if (name.empty() || name.starts_with("comment")) {
                    continue;
                }

                if (++count > MAX_EXPECTED) {
                    out.clear();
                    break;
                }
                if (!out.empty()) {
                    out.append(", ");
                }
                out.append(formatSymbol(name, kind == TSSymbolTypeRegular));
            }

            ts_lookahead_iterator_delete(iter);
            return out;
        }
    }

    Diagnostic buildMissingDiagnostic(ts::Node node, std::string_view source, const File *originFile) {
        Diagnostic diag{.file = originFile};
        const std::string expected = Helper::formatSymbol(node.getType(), node.isNamed());

        if (const std::optional<ts::Node> prev = Helper::findPrevSibling(node); prev.has_value()) {
            const Position anchor = Helper::toPosition(prev->getPointRange().end);
            diag.range = Range{.start = anchor, .end = anchor};

            if (const std::string after = Helper::extractSnippet(Helper::lastLeaf(*prev), source); !after.empty()) {
                USE_CODE(diag, Error, 200004, expected, after);
                return diag;
            }
        } else {
            diag.range = Helper::toRange(node);
        }

        USE_CODE(diag, Error, 200003, expected);
        return diag;
    }

    Diagnostic buildUnexpectedDiagnostic(ts::Node node, std::string_view source, const File *originFile) {
        Diagnostic diag{.file = originFile, .range = Helper::toRange(node)};
        const std::string found = Helper::extractSnippet(node, source);
        if (found.empty()) {
            USE_CODE(diag, Error, 200005);
            return diag;
        }

        const std::string expected = Helper::expectedFromState(node.getLanguage(), ts_node_parse_state(node.impl));
        if (!expected.empty()) {
            USE_CODE(diag, Error, 200007, found, expected);
        } else {
            USE_CODE(diag, Error, 200006, found);
        }
        return diag;
    }

    Diagnostic buildErrorDiagnostic(ts::Node node, std::string_view source, const File *originFile) {
        const ts::Node leaf = Helper::firstLeaf(node);
        if (leaf.isMissing()) {
            return buildMissingDiagnostic(leaf, source, originFile);
        }
        return buildUnexpectedDiagnostic(leaf, source, originFile);
    }
}
