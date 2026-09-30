/**
 * @brief
 * Juggernyaut Language Server
**/

// Basic C++ headers
#include <fstream>
#include <exception>
#include <iostream>
#include <variant>
#include <charconv>

// Common headers
#include "common/headers.hpp"

// lsp-framework
#include "lspFramework.hpp"

// Base
#include "base/info.hpp"

// Capabilities
#include "capabilities/basic.hpp"
#include "capabilities/semantics/diagnostics.hpp"

// Store
#include "store/DocumentStore.hpp"

// Session
#include "../core/session/session.hpp"

void logStdErr(std::string_view message) {
    std::cerr << message << std::endl;
}

// This is where the server's setup begins
int startServer(lsp::io::Stream& io) {
    int exit_code = false;
    lsp::ServerEndpoint endpoint = io;

    // Setup session
    Session::Session session = Session::getSessionDefaults();
    Store::DocumentStore store = Store::DocumentStore();
    session.store = &store;

    // Configure protocol
    Capabilities::configureProtocol(endpoint, session, exit_code);
    Capabilities::Semantics::setupGlobalDiagnostics(endpoint, session);

    // Start the communication
    endpoint.runMessageLoop();

    return exit_code;
}

int runStdioServer() {
    return startServer(lsp::io::standardIO());
}

int runSocketServer(unsigned short port) {
    auto listener = lsp::io::SocketListener(port);
    logStdErr("listening for incoming connections on port " + std::to_string(listener.port()));

    while(listener.isOpen())
    {
        auto socket = listener.accept();

        if(!socket.isOpen())
            return EXIT_FAILURE;

        std::thread(
            [socket = std::move(socket)]() mutable
            {
                logStdErr("client connected");

                startServer(socket); // The returned value is ignored!

                logStdErr("client disconnected");
            }).detach();
    }

    return EXIT_SUCCESS;
}

std::optional<unsigned short> parsePortArg(int argc, char** argv) {
    constexpr auto PortArg = std::string_view("--port=");

    for(int i = 1; i < argc; ++i) {
        const auto arg = std::string_view(argv[i]);

        if(!arg.starts_with(PortArg)) {
            logStdErr("ignoring unknown argument '" + std::string(arg) + '\'');
            continue;
        }

        const auto portStr = arg.substr(PortArg.size());
        unsigned short port = 0;
        const auto [ptr, ec] = std::from_chars(portStr.data(), portStr.data() + portStr.size(), port);
        (void)ptr;

        if(ec == std::errc{})
            return port;

        logStdErr("invalid port '" + std::string(portStr) + '\'');
    }

    return std::nullopt;
}

int main(int argc, char** argv) {
    try {
        if(const auto port = parsePortArg(argc, argv))
            return runSocketServer(*port);

        logStdErr("starting stdio server - pass '--port=<port>' for a socket server");
        return runStdioServer();
    } catch(const std::exception& e) {
        logStdErr(e.what());

        return EXIT_FAILURE;
    }
}
