/**
 * @brief 
 * Parser implementations
**/

#pragma once

#include "common/headers.hpp"
#include "dynamic.hpp" // JUG_PARSER_API

// Data & Diagnostics
#include "../data/host/VFS.hpp"
#include "../data/parser/SharedCSTCache.hpp"
#include "../data/parser/AST.hpp"
#include "../data/diagnostics/Diagnostic.hpp"

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
    // Host
    using File = Data::Host::File;
    using Range = Data::Diagnostics::Range;
    using Diagnostic = Data::Diagnostics::Diagnostic;

    // Driver
    using Environment = Data::Driver::Environment;

    // Parser
    using SharedCSTCache = Data::Parser::SharedCSTCache;
    using InternalCSTHandle = Data::Parser::CSTHandle;
    using AST = Data::Parser::AST;
    

    struct ImportRequest {
        std::string target = "";
        std::string alias = "";
        std::vector<std::string> symbols;
        Range range{};
        bool isExternal = false; // 'true' only when importing libraries!
    };

    struct ParseResult {
        std::unique_ptr<AST> ast = nullptr;
        std::vector<ImportRequest> imports;
        bool hasSyntaxErrors = false;
    };

    class JUG_PARSER_API Parser {
        public:
            Configs configs{};
            Hooks hooks{};
        private:
            const Environment *env = nullptr;
            SharedCSTCache *cstCache = nullptr;

            void emitDiagnostic(const File *file, Diagnostic &diag) const;
            void sendCSTDataReport(const File *file, std::string &&data) const;
            
            // tree-sitter
            InternalCSTHandle updateCST(const File *file) const;
            void negotiateCSTCache(const File *file, const InternalCSTHandle &cst) const;

        public:
            Parser(Configs parserConfigs = {}, Hooks parserHooks = {}, const Environment *sharedEnvironment = nullptr, SharedCSTCache *sharedCstCache = nullptr)
                : configs(std::move(parserConfigs)), hooks(std::move(parserHooks)), env(sharedEnvironment), cstCache(sharedCstCache) {}
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

            // Clear shared caches
            static void cleanup();
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
