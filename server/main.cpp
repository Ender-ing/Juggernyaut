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
#include "../core/parser/listeners/errors.hpp"

int main(int argc, const char *argv[]) {
    // Test for memory leaks
    Common::CrtDebug::initiateCrtMemoryChecks();

    // Initalise communications protocol
    try {
        auto connection = lsp::Connection(lsp::io::standardIO());
        auto messageHandler = lsp::MessageHandler(connection);

        bool received_shutdown = false;
        Store::DocumentStore store;

        Capabilities::configureProtocol(messageHandler, received_shutdown, store);

        while(true) {
            messageHandler.processIncomingMessages();
        }
    } catch(const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    // exit() didn't work??
    return 1;
}

void exit(const int &code) {
    // Handle memory check results
    if(Common::CrtDebug::processCrtMemoryReports()){
        // Exist with an error on memory leaks!
        // "Terminating program due to detected memory errors! Please contact the developers of Juggernyaut!"
        std::exit(1);
    }

    // Terminate the process!
    std::exit(code);
}
