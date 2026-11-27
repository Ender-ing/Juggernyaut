/**
 * @brief
 * Jug literal decoding, validation, and lowering rules
**/

#include "literals.hpp"

namespace Parser::Rules::Base {
    std::string unescapeStringLiteral(std::string_view raw) {
        if (raw.size() >= 2u && raw.front() == '"' && raw.back() == '"') {
            raw = raw.substr(1u, raw.size() - 2u);
        }
        if (raw.find('\\') == std::string_view::npos) {
            return std::string(raw);
        }

        std::string out;
        out.reserve(raw.size());
        for (size_t i = 0u; i < raw.size(); ++i) {
            if (raw[i] != '\\') {
                out.push_back(raw[i]);
                continue;
            }
            if (i + 1u >= raw.size()) {
                out.push_back('\\');
                break;
            }
            switch (raw[++i]) {
                case 'n':  out.push_back('\n'); break;
                case 'r':  out.push_back('\r'); break;
                case 't':  out.push_back('\t'); break;
                case '0':  out.push_back('\0'); break;
                case '"':  out.push_back('"');  break;
                case '\'': out.push_back('\''); break;
                case 'b':  out.push_back('\b'); break;
                case 'f':  out.push_back('\f'); break;
                case 'v':  out.push_back('\v'); break;
                case '\\': out.push_back('\\'); break;
                case '\n': break; // Line continuation (`\\\n`)
                case '\r':        // Line continuation (`\\\r\n`)
                    if (i + 1u < raw.size() && raw[i + 1u] == '\n') {
                        ++i;
                    }
                    break;
                default:
                    out.push_back(raw[i]);
                    break;
            }
        }
        return out;
    }

    void registerLiterals(Lowering::LoweringTable &table) {
        table.bindLeaf("lit_string", Data::Parser::ASTKind::StringLiteral);
        // TO-DO: `lit_boolean`, `lit_int`, `lit_float`, `lit_num_rep`, `lit_char`, `lit_array`
    }
}
