/**
 * @brief
 * Juggernyaut Language Server
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

// Base
#include "base/info.hpp"

// Store
#include "store/DocumentStore.hpp"

// Capabilities
#include "capabilities/print.hpp"
#include "capabilities/basic.hpp"
#include "capabilities/semantics/diagnostics.hpp"

// Parser
#include "../core/parser/parser.hpp"
#include "../core/parser/listeners/DiagnosticListener.hpp"

int main(int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    int exit_code = false;

    // Initalise communications protocol
    try {
        auto connection = lsp::Connection(lsp::io::standardIO());
        auto messageHandler = lsp::MessageHandler(connection);

        Store::DocumentStore store;

        Capabilities::configureProtocol(messageHandler, store, exit_code);

        while(true) {
            messageHandler.processIncomingMessages();
        }
    } catch(const std::exception& e) {
        exit_code = 1;
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    // exit() didn't work??
    return exit_code;
}
