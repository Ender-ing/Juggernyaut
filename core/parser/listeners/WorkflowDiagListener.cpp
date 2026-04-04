/**
 * @brief
 * Session Workflow listeners
**/

#include "WorkflowDiagListener.hpp"

namespace Parser {
    namespace Listeners {
        void WorkflowDiagListener::onSyntaxError(Diagnostics::Diagnostic diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = source->getId();

            // Trigger Events
            const ParserDiagEvent &call = this->hooks.onSyntaxError;
            if (call != nullptr) {
                call(diag);
            }

            // Push the diagnostic
            source->addParserDiagnostic(std::move(diag));
        }
        void WorkflowDiagListener::onAmbiguity(Diagnostics::Diagnostic diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = source->getId();

            // Trigger Events
            const ParserDiagEvent &call = this->hooks.onAmbiguity;
            if (call != nullptr) {
                call(diag);
            }

            // Push the diagnostic
            source->addParserDiagnostic(std::move(diag));
        }
        void WorkflowDiagListener::onAttemptingFullContext(Diagnostics::Diagnostic diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = source->getId();

            // Trigger Events
            const ParserDiagEvent &call = this->hooks.onAttemptingFullContext;
            if (call != nullptr) {
                call(diag);
            }

            // Push the diagnostic
            source->addParserDiagnostic(std::move(diag));
        }
        void WorkflowDiagListener::onContextSensitivity(Diagnostics::Diagnostic diag) {
            std::unique_ptr<Data::Store::Source> &source = this->src;

            diag.sourceId = source->getId();

            // Trigger Events
            const ParserDiagEvent &call = this->hooks.onContextSensitivity;
            if (call != nullptr) {
                call(diag);
            }

            // Push the diagnostic
            source->addParserDiagnostic(std::move(diag));
        }
    }
}