/**
 * @brief
 * Handle .toml workspace configs
**/

#include "workspace.hpp"

namespace Configs {
    namespace Workspace {
        namespace Internal {
            void tomlNodeRange(toml::node &node, Diagnostics::Diagnostic &diag) {
                auto &src = node.source();

                diag.range.start.line = static_cast<uint32_t>((int) src.begin.line - 1);
                diag.range.start.character = static_cast<uint32_t>((int) src.begin.column - 1);
                diag.range.end.line = static_cast<uint32_t>((int) src.end.line - 1);
                diag.range.end.character = static_cast<uint32_t>((int) src.end.column - 1);
            }
        }
        BreakingChanges importDirs(Session::Session &session, std::vector<Diagnostics::Diagnostic> &diags, bool isStrict,
            toml::table &workspace, const std::string &rootUri) {
            BreakingChanges breaking = BreakingChanges::None;

            auto importDirsNode = workspace["importDirs"];
            if (importDirsNode.is_array()) {
                toml::array *dirs = importDirsNode.as_array();

                // Signal danger
                breaking = BreakingChanges::Parser;

                if (isStrict) {
                    session.store->resetImportDirs();
                }

                for (toml::node& dir : *dirs) {
                    if (dir.is_string()) {
                        std::string dirStr = dir.value_or("");

                        if (dirStr == "" || !(session.store->_isDirValid(dirStr))) {
                            Diagnostics::Diagnostic diag;
                            diag.severity = Diagnostics::Severity::Error;
                            diag.message = CODE_600003;
                            diag.code = 600003;

                            // Range
                            Internal::tomlNodeRange(dir, diag);

                            diags.push_back(std::move(diag));
                        } else {
                            std::string absPath = session.store->_joinPaths(rootUri, dirStr);
                            session.store->addImportDir(absPath);
                        }
                    }
                }
            }

            return breaking;
        }
    }
}
