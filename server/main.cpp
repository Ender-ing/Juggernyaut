/**
 * @brief
 * Juggernyaut Language Server
**/

// Basic C++ headers
#include <fstream>
#include <exception>
#include <iostream>
#include <variant>

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "lspFramework.hpp"

// Base
#include "base/info.hpp"

// Capabilities
#include "capabilities/print.hpp"
#include "capabilities/basic.hpp"
#include "capabilities/semantics/diagnostics.hpp"

// Store
#include "store/DocumentStore.hpp"

// Session
#include "../core/session/session.hpp"

int main(int argc, const char *argv[]) {
    int exit_code = false;

    // Setup session
    Session::Session session = Session::getSessionDefaults();
    Store::DocumentStore store;
    session.store = &store;

    // Initalise protocol
    try {
        auto connection = lsp::Connection(lsp::io::standardIO());
        auto messageHandler = lsp::MessageHandler(connection);

        Capabilities::configureProtocol(messageHandler, session, exit_code);
        Capabilities::Semantics::setupParserDiagnostics(messageHandler, session);

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
