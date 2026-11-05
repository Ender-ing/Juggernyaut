/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Data & Diagnostics
#include "../data/host/VFS.hpp"
#include "../data/parser/AST.hpp"
#include "../diagnostics/Diagnostic.hpp"

// Environment
#include "./../data/driver/Environment.hpp"

// Parser
#include "Hooks.hpp"
#include "Configs.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // Suppress DLL interface warning for STL types
#endif

namespace Parser {
    using File = Data::Host::File;
    using Range = Diagnostics::Range;
    using Setup = Data::Driver::Environment;
    using ASTNode = Data::Parser::ASTNode;

    struct ImportRequest {
        bool isExternal = false; // 'true' only when importing libraries!
        std::string target = "";
        std::string alias = "";
        std::vector<std::string> symbols;
        Range range{};
    };

    struct ParseResult {
        std::unique_ptr<ASTNode> ast = nullptr;
        std::vector<ImportRequest> imports;
        bool hasSyntaxErrors = false;
    };

    class JUG_PARSER_API Parser {
        private:
            const Setup *setup;
            Configs configs{};
            Hooks hooks{};

            void emitDiagnostic(const File *file, Diagnostics::Diagnostic &diag) const;
            void reportAntlrData(const File *file, std::string &&data) const;

        public:
            Parser(const Setup *sharedSetup, Configs parserConfigs = {}, Hooks parserHooks = {})
                : setup(sharedSetup), configs(std::move(parserConfigs)), hooks(std::move(parserHooks)) {}
            ~Parser() = default;

            // Non-copyable, movable
            Parser(const Parser&) = delete;
            Parser& operator=(const Parser&) = delete;
            Parser(Parser&&) noexcept = default;
            Parser& operator=(Parser&&) noexcept = default;

            // Configuration & Hook accessors
            Configs& getConfigs() noexcept { return this->configs; }
            const Configs& getConfigs() const noexcept { return this->configs; }
            Hooks& getHooks() noexcept { return this->hooks; }
            const Hooks& getHooks() const noexcept { return this->hooks; }

            // Parse a single VFS file into an AST and list of ImportRequests
            ParseResult parse(const File *file) const;

            // Clear shared ANTLR4 DFA caches
            static void cleanup();
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
