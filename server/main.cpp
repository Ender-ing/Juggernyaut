/**
 * @brief
 * Juggernyaut Compiler
**/

// Basic C++ headers
#include <fstream>
#include <exception>
#include <iostream>
#include <future>
#include <chrono>
#include <variant>

// Common headers
#include "../core/common/headers.hpp"
#include "../core/common/debug.hpp"

// lsp-framework
#include "lspFramework.hpp"

// Parser
#include "../core/parser/parser.hpp"
#include "../core/parser/listeners/errors.hpp"

// TMP
// Listen for syntax-related errors
class SyntaxErrorListener : public Parser::Listeners::ErrorListener {
    private:
        // (Storing a std::string value will result in a C4251 MSVC warning)
        const char *stage; // "Lexer" or "Parser"
        std::vector<lsp::Diagnostic> &diags;
    public:
        // Constructors
        SyntaxErrorListener(const char *stageName, std::vector<lsp::Diagnostic> &diagsVector) : stage(stageName), diags(diagsVector) { }
        SyntaxErrorListener(const std::string &stageName, std::vector<lsp::Diagnostic> &diagsVector) : stage(stageName.c_str()), diags(diagsVector) {}

        // ANTLR4 functions
        void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
        size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {

            lsp::Diagnostic error;

            // Get the starting position
            error.range.start.line = (uint32_t) (line - 1);
            error.range.start.character = (uint32_t) charPositionInLine;

            // Determine the end position
            std::string tokenText;
            if (offendingSymbol) {
                // TMP
                tokenText = offendingSymbol->getText();
                error.range.end.character = (uint32_t) (charPositionInLine + tokenText.length());
            } else {
                // TMP
                tokenText = "<N/A>";
                error.range.end.character = error.range.start.character + 1;
            }
            error.range.end.line = error.range.start.line; // No token is allowed to contain newlines!

            error.severity = lsp::DiagnosticSeverity::Error;

            //error.message = (std::string) this->stage + " Error: " + msg + "\n(Token Text: '" + tokenText + "')";
            error.message = (std::string) this->stage + " Error: " + msg;
            error.source = "Juggernyaut";
            this->diags.push_back(error);
        }
        void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs)
            override {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Ambiguity reported from index ",
            //     startIndex ," to index " , stopIndex, Console::END_REPORT);
        }
        void reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
            antlr4::atn::ATNConfigSet *configs) override {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Attempting full context reported from index ",
            //     startIndex ," to index " , stopIndex, Console::END_REPORT);
        }
        void reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, size_t prediction,
            antlr4::atn::ATNConfigSet *configs) override {

            // REPORT(Console::START_REPORT, Console::CRITICAL_REPORT, "Context sensitivity reported from index ",
            //         startIndex ," to index " , stopIndex, Console::END_REPORT);
        }
};

void validateDocumentSyntax(lsp::MessageHandler &messageHandler, const lsp::DocumentUri &documentUri, const std::string &latestSourceCode) {
    // TMP
    std::vector<lsp::Diagnostic> diagnostics;

    SyntaxErrorListener lexerDebugErrorListener("Lexer", diagnostics);
    SyntaxErrorListener parserDebugErrorListener("Parser", diagnostics);

    // REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Tokens: \n");
    Parser::Debug::syntaxCheck(latestSourceCode,
        [](const std::string tokenText){
            // REPORT(tokenText, "\n");
        },
        [](const std::string treeText){
            // REPORT(Console::END_REPORT);
            // REPORT(Console::START_REPORT, Console::DEBUG_REPORT, "Parse Tree: \n", treeText, Console::END_REPORT);
        }, &lexerDebugErrorListener, &parserDebugErrorListener);

    // Publish the diagnostics to the editor
    auto diagParams = lsp::notifications::TextDocument_PublishDiagnostics::Params{};
    diagParams.uri = documentUri; // The URI of the file you just checked
    diagParams.diagnostics = diagnostics;
    messageHandler.sendNotification<lsp::notifications::TextDocument_PublishDiagnostics>(std::move(diagParams));
}


template<typename MessageType>
void printMessageMethod() {
    const auto type = lsp::message::IsNotification<MessageType> ? "notification" : "request";
    std::cerr << "Server received " << type << " '" << MessageType::Method << '\'' << std::endl;
}

template<typename MessageType>
void printMessagePayload (const typename MessageType::Params& params) {
    const auto json = lsp::toJson(typename MessageType::Params(params));
    std::cerr << "payload: " << lsp::json::stringify(json, true) << std::endl;
}

template<typename MessageType>
void printMessage (const typename MessageType::Params& params) {
    printMessageMethod<MessageType>();
    printMessagePayload<MessageType>(params);
}

template<typename MessageType>
void printMessage () {
    printMessageMethod<MessageType>();
}

void registerCallbacks (lsp::MessageHandler &messageHandler, bool &is_running) {
    messageHandler.add<lsp::requests::Initialize>(
        [](lsp::requests::Initialize::Params&& params) {
            printMessage<lsp::requests::Initialize>(params);

            /*
             * Respond with an InitializeResult containing some basic server info and capabilities
             */

            return lsp::requests::Initialize::Result {
                .capabilities = {
                    .positionEncoding = lsp::PositionEncodingKind::UTF16,
                    .textDocumentSync = lsp::TextDocumentSyncOptions {
                        .openClose = true,
                        .change = lsp::TextDocumentSyncKind::Full,
                        .save = true
                    },
                    .hoverProvider = false,
                },
                .serverInfo = lsp::InitializeResultServerInfo {
                    .name = "Juggernyaut Language Server",
                    .version = MAIN_TARGET_BINARY_VERSION
                },
            };
        }
    ).add<lsp::requests::Shutdown>(
        []() {
            printMessage<lsp::requests::Shutdown>();
            return lsp::requests::Shutdown::Result();
        }
    ).add<lsp::notifications::Exit>(
        [&is_running]() {
            printMessage<lsp::notifications::Exit>();
            is_running = false;
        }
    )
    // Listen for when a file is opened
    .add<lsp::notifications::TextDocument_DidOpen>(
        [&messageHandler](lsp::notifications::TextDocument_DidOpen::Params&& params) {
            const lsp::DocumentUri documentUri = params.textDocument.uri;
            const std::string sourceCode = params.textDocument.text;

            validateDocumentSyntax(messageHandler, documentUri, sourceCode);
        }
    )
    // Listen for when a file is modified
    .add<lsp::notifications::TextDocument_DidChange>(
        [&messageHandler](lsp::notifications::TextDocument_DidChange::Params&& params) {
            const lsp::DocumentUri documentUri = params.textDocument.uri;
            
            // Note: If you requested Full sync in your InitializeResult, 
            // params.contentChanges[0].text will contain the entire updated file.
            if (!params.contentChanges.empty()) {
                const std::string updatedSourceCode = std::visit(
                    [](const auto& changeEvent) { return changeEvent.text; }, 
                    params.contentChanges[0]
                );
                validateDocumentSyntax(messageHandler, documentUri, updatedSourceCode);
            }
        }
    );
}

int main (int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    // Initalise communications protocol
    try {
        auto connection = lsp::Connection(lsp::io::standardIO());
        auto messageHandler = lsp::MessageHandler(connection);

        bool is_running = true;
        registerCallbacks(messageHandler, is_running);

        while(is_running)
            messageHandler.processIncomingMessages();
    } catch(const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    // Handle memory check results
    if(Common::CrtDebug::processCrtMemoryReports()){
        // Exist with an error on memory leaks!
        // "Terminating program due to detected memory errors! Please contact the developers of Juggernyaut!"
        return 1;
    }

    // End the program
    return 0;
}
