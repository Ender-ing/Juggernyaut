/**
 * @brief
 * Parser implementations
**/

// Parser
#include "parser.hpp"

// ANTLR4 imports
#include "antlr4.hpp"
#include "JuggernyautLexer.h"
#include "JuggernyautParser.h"

#include "internal/JugLexer.hpp"

#include "listeners/WorkflowDiagListener.hpp"

namespace Parser {
    namespace Debug {
        // Check for syntax errors
        // [true -> success, false -> failure]
        bool syntaxCheck(const std::string &file_contents, const TokenReport &onTokenCall, const TreeReport &onTreeCall,
            Listeners::DiagnosticListener *lexerErrorListener, Listeners::DiagnosticListener *parserErrorListener) {
            // Use the file's input
            antlr4::ANTLRInputStream input(file_contents);

            // Tokens
            Internal::JugLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);

            // Parse tree
            GeneratedParser::JuggernyautParser parser(&tokens);

            // Check for syntax errors
            lexer.removeErrorListeners();// remove default parser error listeners.
            lexer.addErrorListener(lexerErrorListener);
            parser.removeErrorListeners();// remove default parser error listeners.
            parser.addErrorListener(parserErrorListener);

            // Pass tokens
            tokens.fill();
            for (auto token : tokens.getTokens()) {
                onTokenCall((const std::string) token->toString());
            }

            // Get the start tree!
            antlr4::tree::ParseTree *tree = parser.prog();

            // Pass the parse tree!
            onTreeCall((const std::string) tree->toStringTree(&parser));

            return Listeners::errorsDetected;
        }
    }

    void contextWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store, std::unique_ptr<Data::Store::Source> &source,
        Listeners::WorkflowDiagListener *listener) ;

    // Visit sources
    void investegateContexts(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store, std::unique_ptr<Data::Store::Source> &source,
        Listeners::WorkflowDiagListener *listener) {
        source->visitDependencies([&configs, &hooks, &store, &listener](Data::Store::SourceID depId){
            std::unique_ptr<Data::Store::Source> &dep = store->getSourceById(depId);
            contextWorkflow(configs, hooks, store, dep, listener);
        });
    }

    // Workflow for individual sources
    void contextWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store, std::unique_ptr<Data::Store::Source> &source,
        Listeners::WorkflowDiagListener *listener) {
        // Check the need for updates
        if (!source->getUpdateAST()) {
            return;
        }

        // Trigger context-level event
        if (hooks.onContextStart != nullptr) {
            hooks.onContextStart();
        }

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
        lexer.addErrorListener(listener);

        // Process tokens
        std::unique_ptr<antlr4::Token> token;
        do {
            token = lexer.nextToken();

            // Trigger events
            if (hooks.onANTLRTokenDetected != nullptr) {
                hooks.onANTLRTokenDetected((const std::string) token->toString());
            }
        } while (token->getType() != antlr4::Token::EOF);

        // Trigger context-level event
        if (hooks.onLexerContextEnd != nullptr) {
            hooks.onLexerContextEnd();
        }
        if (configs.terminateAfterLexer) {
            // Trigger context-level event
            if (hooks.onContextEnd != nullptr) {
                hooks.onContextEnd();
            }
            return;
        }

        // [STAGE] Parser
        // Parser
        GeneratedParser::JuggernyautParser parser(&tokens);

        // Check for syntax errors
        parser.removeErrorListeners(); // Remove default error listeners
        parser.addErrorListener(listener);

        // Get the start tree!
        antlr4::tree::ParseTree *tree = parser.prog();

        // Trigger Events
        if (hooks.onANTLRTreeGenerated != nullptr) {
            hooks.onANTLRTreeGenerated((const std::string) tree->toStringTree(&parser));
        }

        // Generate an AST
        // Note that within this step, import statements will trigger a "context investigation"!
        // ...

        // Attach AST data to <Source>
        // ...

        // Trigger context-level event
        if (hooks.onContextEnd != nullptr) {
            hooks.onContextEnd();
        }

        // Visit dependencies
        investegateContexts(configs, hooks, store, source, listener);
    }

    // Triggered by Session::initiate
    void sessionWorkflow(const Configs &configs, const Hooks &hooks, Data::Store::SourceStore *store) {
        Listeners::WorkflowDiagListener listener = Listeners::WorkflowDiagListener(hooks);

        store->visitEntries([&configs, &hooks, &store, &listener](const Data::Store::SourceID entryID) {
            // Get source object
            std::unique_ptr<Data::Store::Source> &src = store->getSourceById(entryID);

            contextWorkflow(configs, hooks, store, src, &listener);
        });
    }
}
