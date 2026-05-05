/**
 * @brief
 * Handle .toml configs
**/

#include "configs.hpp"
#include "workspace.hpp"

// toml++
#include "tomlpp.hpp"

// Diagnostics
#include "DiagCodes.hpp"

namespace Configs {
    namespace Internal {
        std::optional<Diagnostics::Diagnostic> getTable(Session::Session &session, const std::string &configUri, toml::table &table) {
            if (session.store == nullptr) {
                throw std::runtime_error("No Session Store object found!");
            }

            std::string rawContent;
            if (!(session.store->_getRawFile(configUri, rawContent))) {
                Diagnostics::Diagnostic err;
                err.severity = Diagnostics::Severity::Error;
                err.message = CODE_600001(configUri);
                err.code = 600001;

                return err;
            }

            // Process toml data
            toml::parse_result result = toml::parse(rawContent);
            if (!result) {
                const toml::parse_error& err = result.error();
                Diagnostics::Diagnostic diag;

                std::string description = std::string(err.description());

                diag.severity = Diagnostics::Severity::Error;
                diag.message = CODE_600002(description);
                diag.code = 600002;

                // Get range
                const auto errSrc = err.source();
                diag.range.start.line = static_cast<uint32_t>((int) errSrc.begin.line - 1);
                diag.range.start.character = static_cast<uint32_t>((int) errSrc.begin.column - 1);
                diag.range.end.line = static_cast<uint32_t>((int) errSrc.end.line - 1);
                diag.range.end.character = static_cast<uint32_t>((int) errSrc.end.column - 1);

                return diag;
            }
            rawContent.clear();
            rawContent.shrink_to_fit();

            table = std::move(result).table();
            return std::nullopt;
        }

        // Restoring sessions
        BreakingChanges chronoSnapshot = BreakingChanges::None;
        constexpr void trackChanges(BreakingChanges *track, const BreakingChanges &change) {
            if (change < chronoSnapshot) {
                chronoSnapshot = change;
            }
            if (track != nullptr && change < (*track)) {
                (*track) = change;
            }
        }
    }
    bool modifySession(Session::Session &session, const std::string &configUri, std::vector<Diagnostics::Diagnostic> &diags, bool isStrict, BreakingChanges *breaking) {
        toml::table configs;
        const std::string rootUri = session.store->_getPathDir(configUri);

        std::optional<Diagnostics::Diagnostic> error = Internal::getTable(session, configUri, configs);
        if (error.has_value()) {
            diags.push_back(error.value());
            return false;
        }

        if (toml::table *workspaceTable = configs["workspace"].as_table()) {
            Internal::trackChanges(breaking, Workspace::importDirs(session, diags, isStrict, *workspaceTable, rootUri));
        }

        return true;
    }
    BreakingChanges resetSessionConfigs(Session::Session &session) {
        BreakingChanges changes = Internal::chronoSnapshot;

        switch (changes) {
        case BreakingChanges::Parser:
            // Reset <SourceStore> configs
            session.store->resetImportDirs();;
        case BreakingChanges::None:
        default:
            break;
        }

        // Reset snapshot
        Internal::chronoSnapshot = BreakingChanges::None;

        return changes;
    }
    void refreshSessionState(Session::Session &session, BreakingChanges changes) {
        switch (changes) {
        case BreakingChanges::Parser:
            session.states.parser++;
        case BreakingChanges::None:
        default:
            break;
        case BreakingChanges::INVALID:
            throw std::runtime_error("Invalid Configs::BreakingChanges value detected!");
            break;
        }
    }
}
