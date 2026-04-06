/**
 * @brief
 * Parser implementations
**/

// Parser
#include "parser.hpp"

// Internal
#include "internal/JugLexer.hpp"
#include "internal/JugParser.hpp"

// Listeners
#include "listeners/WorkflowDiagListener.hpp"
#include "listeners/ASTGenListener.hpp"

// Visitors
#include "visitors/ASTGenVisitor.hpp"

namespace Parser {
    void contextWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store,
        std::unique_ptr<Data::Store::Source> &source) ;

    // Visit sources
    void investegateContexts(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store,
        std::unique_ptr<Data::Store::Source> &source) {
        source->visitDependencies([&configs, &hooks, &store](Data::Store::SourceId depId){
            std::unique_ptr<Data::Store::Source> &dep = store->getSourceById(depId);
            contextWorkflow(configs, hooks, store, dep);
        });
    }

    // Workflow for individual sources
    void contextWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store,
        std::unique_ptr<Data::Store::Source> &source) {
        // Check the need for updates
        if (!(source->getUpdateAST())) {
            return;
        }

        // Reset diagnostics
        source->resetParserDiagnostics();

        // Trigger context-level event
        if (hooks.onContextStart != nullptr) {
            hooks.onContextStart(source->getId());
        }

        Listeners::WorkflowDiagListener listener = Listeners::WorkflowDiagListener(hooks, source);

        // Get raw content
        const std::string &rawContent = source->getRawContent();

        // [STAGE] Lexer
        // Use the file's input
        antlr4::ANTLRInputStream input(rawContent);

        // Lexer & Tokens
        Internal::JugLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);

        // Check for syntax errors
        lexer.removeErrorListeners(); // Remove default error listeners
        lexer.addErrorListener(&listener);

        // Process tokens
        if (hooks.onANTLRTokenDetected == nullptr) {
            tokens.fill();
        } else {
            antlr4::Token *token = nullptr;
            do {
                // Look ahead and store
                token = tokens.LT(1);

                // Trigger events
                hooks.onANTLRTokenDetected(token->toString());
    
                // Advance
                if (token->getType() != antlr4::Token::EOF) {
                    tokens.consume();
                }
            } while (token->getType() != antlr4::Token::EOF);
            // Rewind
            tokens.seek(0);
        }

        // Trigger context-level event
        if (hooks.onLexerContextEnd != nullptr) {
            hooks.onLexerContextEnd(source->getId());
        }
        if (configs.terminateAfterLexer) {
            // Trigger context-level event
            if (hooks.onContextEnd != nullptr) {
                hooks.onContextEnd(source->getId());
            }
            return;
        }

        // [STAGE] Parser
        // Parser
        Internal::JugParser parser(&tokens);

        // Check for syntax errors
        parser.removeErrorListeners(); // Remove default error listeners
        parser.addErrorListener(&listener);

        // Get the start tree!
        antlr4::tree::ParseTree *tree = parser.prog();

        // Trigger Events
        if (hooks.onANTLRTreeGenerated != nullptr) {
            hooks.onANTLRTreeGenerated(tree->toStringTree(&parser));
        }

        // Generate an AST
        // Note that within this step, import statements will trigger a "context investigation"!
        Listeners::ASTGenListener astListener = Listeners::ASTGenListener(source);
        Visitors::ASTGenVisitor visitor = Visitors::ASTGenVisitor(&astListener, store, source);
        visitor.visit(tree);

        // Attach AST data to <Source>
        // ...

        // Trigger context-level event
        if (hooks.onContextEnd != nullptr) {
            hooks.onContextEnd(source->getId());
        }

        // Flag AST as "up-to-date"
        source->preventUpdateAST();

        // Visit dependencies
        investegateContexts(configs, hooks, store, source);
    }

    // Triggered by Session::initiate
    void sessionWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store) {
        store->visitEntries([&configs, &hooks, &store](const Data::Store::SourceId entryID) {
            // Get source object
            std::unique_ptr<Data::Store::Source> &src = store->getSourceById(entryID);

            contextWorkflow(configs, hooks, store, src);
        });
    }
}
