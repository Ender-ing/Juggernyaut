/**
 * @brief
 * Session Workflow listeners
**/

#pragma once

#include "DiagnosticListener.hpp"

// Parser
#include "../Hooks.hpp"

#include "../../data/store/SourceStore.hpp"

namespace Parser {
    namespace Listeners {
        // Listen for syntax-related errors
        class JUG_PARSER_API WorkflowDiagListener : public DiagnosticListener {
            private:
                const Hooks &hooks;
                std::unique_ptr<Data::Store::Source> &src;
            public:
                // Constructors
                WorkflowDiagListener(const Hooks &parserHooks, std::unique_ptr<Data::Store::Source> &source)
                    : hooks(parserHooks), src(source) {};

                void onSyntaxError(Diagnostics::Diagnostic diag) override;
                void onAmbiguity(Diagnostics::Diagnostic diag) override;
                void onAttemptingFullContext(Diagnostics::Diagnostic diag) override;
                void onContextSensitivity(Diagnostics::Diagnostic diag) override;
        };
    }
}
