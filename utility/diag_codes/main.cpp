/**
 * @brief
 * Manage diagnostic code generation
**/

// Basic C++ headers
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <algorithm>
#include <cctype>

// ANTLR runtime
#include "antlr4-runtime.h"
#include "DiagnosticsCodeLexer.h"
#include "DiagnosticsCodeParser.h"
#include "DiagnosticsCodeParserVisitor.h"

using namespace antlr4;
using namespace GeneratedParser;
namespace fs = std::filesystem;

#define ERR \
std::cerr << "\033[31m" << "ERROR: "

#define ERR_END \
"\033[0m\n"

#define IN_VECTOR(VECTOR, TARGET) \
(std::find(VECTOR.begin(), VECTOR.end(), TARGET) != VECTOR.end())

const bool debug = false;

bool isDigit(const std::string& str) {
    if (str.empty()) return false;

    return std::all_of(str.begin(), str.end(), [](unsigned char c) {
        return std::isdigit(c);
    });
}

std::string escapeChar(const std::string &text, char target) {
    std::string escapedText;
    
    // Alocate a minimum
    escapedText.reserve(text.size()); 
    
    for (char c : text) {
        if (c == target) {
            escapedText.push_back('\\');
        }
        escapedText.push_back(c);
    }
    
    return escapedText;
}

// Note: You can specify a return type, e.g., <std::any> or <int>
class Visitor : public DiagnosticsCodeParserVisitor {
    private:
        DiagnosticsCodeParser* parser;
        const std::string &srcPath;
        const int zoneId;
    public:
        Visitor(DiagnosticsCodeParser* parser, const std::string &srcPath, int zoneId)
            : parser(parser), srcPath(srcPath), zoneId(zoneId) {}
        antlrcpp::Any visitList(DiagnosticsCodeParser::ListContext *context) override {
            std::ostringstream out;
            out << "// [START] SRC: " << srcPath << "\n";

            auto codeDefs = context->code();
            for (auto codeDef : codeDefs) {
                out << any_cast<std::string>(visit(codeDef));
            }
            out << "// [END] SRC: " << srcPath << "\n\n";

            return out.str();
        }

        // Diagnostic code
        uint32_t code;
        bool isCodeStatic = false;
        std::vector<uint32_t> codes;
        std::ostringstream codeContent;
        bool appendSpace = false;
        antlrcpp::Any visitCode(DiagnosticsCodeParser::CodeContext *context) override {
            std::ostringstream out;
            auto id = context->ID();
            if (id == nullptr) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "expecting valid code id",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(MISSING_CODE_ID)");
            }
            code = static_cast<uint32_t>(std::stoul(id->getText()));
            if (code > 99999) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "code id must be smaller than 99999",
                    nullptr // We don't need to pass a specific exception object here
                );
            }
            code = this->zoneId*100000 + code;
            codeContent.str("");
            codeContent.clear();
            appendSpace = false;
            
            if (IN_VECTOR(codes, code)) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "reused diagnostic code id",
                    nullptr // We don't need to pass a specific exception object here
                );
            }
            codes.push_back(code);
            out << "// Diagnostic code: " << code << "\n";

            auto func = context->function();
            auto sttc = context->static_();
            if ((func == nullptr && sttc == nullptr) || (func != nullptr && sttc != nullptr)) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "expecting a valid function/static definition",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(MISSING_VALID_CODE_DEF)");
            }
            if (func != nullptr) {
                out << any_cast<std::string>(visit(func));
            }
            if (sttc != nullptr) {
                out << any_cast<std::string>(visit(sttc));
            }

            return out.str();
        }

        // Static code
        antlrcpp::Any visitStatic(DiagnosticsCodeParser::StaticContext *context) override {
            std::ostringstream out;

            // Lines
            auto lines = context->line();
            for (auto line : lines) {
                visit(line);
            }

            out << "#define CODE_" << code << " \"";
            out << codeContent.str() << "\"\n";

            return out.str();
        }

        // Code function
        std::vector<std::string> usedVarNames;
        std::vector<std::string> varCalls;
        antlrcpp::Any visitFunction(DiagnosticsCodeParser::FunctionContext *context) override {
            std::ostringstream out;
            auto prs = this->parser;
            auto fail = [&prs, &context](){
                prs->notifyErrorListeners(
                    context->getStart(), 
                    "invalid function definition syntax",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(INVALID_FUNCTION_WRAP)");
            };
            isCodeStatic = false;
            usedVarNames.clear();
            varCalls.clear();

            auto sugar = context->SYM_BRACKET_LEFT();
            if (sugar == nullptr) {
                return fail();
            }

            out << "static FORCE_INLINE std::string CODE_" << code;

            // Vars
            auto varDefs = context->variable_def();
            out << "(";
            bool printComma = false;
            for (auto varDef : varDefs) {
                if (printComma) {
                    out << ", ";
                } else {
                    printComma = true;
                }
                std::string var = any_cast<std::string>(visit(varDef));
                out << var;
            }
            out << ")";

            sugar = context->SYM_BRACKET_RIGHT();
            if (sugar == nullptr) {
                return fail();
            }
            sugar = context->SYM_COLON();
            if (sugar == nullptr) {
                return fail();
            }

            // Lines
            codeContent << "\"";
            auto lines = context->line();
            for (auto line : lines) {
                visit(line);
            }
            codeContent << "\"";

            for (auto varName : varCalls) {
                codeContent << ", " << varName;
            }

            out << " {\n";
            out << "    return std::format(";
            out <<      codeContent.str() << ");\n";
            out << "}\n";

            return out.str();
        }

        // Var
        antlrcpp::Any visitVariable_def(DiagnosticsCodeParser::Variable_defContext *context) override {
            std::ostringstream out;
            auto varType = context->variable_type();
            if (varType == nullptr) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "expecting a valid variable type",
                    nullptr // We don't need to pass a specific exception object here
                );
                out << "(INVALID_VARDEF_TYPE)";
            } else {
                out << any_cast<std::string>(visit(varType));
            }

            out << " ";
            auto varName = context->VAR();
            if (varName == nullptr) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "expecting a valid variable name",
                    nullptr // We don't need to pass a specific exception object here
                );
                out << std::string("(INVALID_VARDEF_NAME)");
            } else {
                std::string name = varName->getText();
                if (IN_VECTOR(usedVarNames, name)) {
                    this->parser->notifyErrorListeners(
                        context->getStart(), 
                        "reused variable name within a function definition",
                        nullptr // We don't need to pass a specific exception object here
                    );
                } else {
                    usedVarNames.push_back(name);
                }
                out << name.erase(0, 1);
            }

            return out.str();
        }
        antlrcpp::Any visitVariable_type(DiagnosticsCodeParser::Variable_typeContext *context) override {
            if (context->TYP_NUMBER() != nullptr) {
                return std::string("int");
            } else if (context->TYP_TEXT() != nullptr) {
                return std::string("std::string");
            }
            this->parser->notifyErrorListeners(
                context->getStart(), 
                "unknown variable type", 
                nullptr // We don't need to pass a specific exception object here
            );
            return std::string("(UNKNOWN_TYPE)");
        }

        // Line
        antlrcpp::Any visitLine(DiagnosticsCodeParser::LineContext *context) override {
            auto contents = context->content();

            if (appendSpace) {
                codeContent << " ";
            } else {
                appendSpace = true;
            }

            for (auto content : contents) {
                if (content->LINE_CONTENT() != nullptr) {
                    // Filter content
                    std::string text = content->getText();
                    text = std::move(escapeChar(text, '"'));

                    codeContent << text;
                }

                // Function-only context

                auto escape = content->escape();
                if (escape != nullptr) {
                    codeContent << any_cast<std::string>(visit(escape));
                }

                auto varCall = content->variable_call();
                if (varCall != nullptr) {
                    codeContent << "{}";
                    varCalls.push_back(any_cast<std::string>(visit(varCall)));
                }
            }

            return nullptr;
        }
        antlrcpp::Any visitContent(DiagnosticsCodeParser::ContentContext *context) override {
            return visitChildren(context);
        }
        // Function-only
        antlrcpp::Any visitEscape(DiagnosticsCodeParser::EscapeContext *context) override {
            if (isCodeStatic) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "unexpected escape sequence",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(ILLEGAL_ESCAPE)");
            }
            if (context->LINE_CALL_ESCAPE()) {
                return std::string("$");
            } else if (context->LINE_SLASH_ESCAPE()) {
                return std::string("\\\\");
            }
            return std::string("(INVALID_ESCAPE)");
        }
        antlrcpp::Any visitVariable_call(DiagnosticsCodeParser::Variable_callContext *context) override {
            if (isCodeStatic) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "unexpected variable reference",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(ILLEGAL_VAR_REF)");
            }
            auto varName = context->LINE_CALL();
            if (varName == nullptr) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "expecting a variable reference",
                    nullptr // We don't need to pass a specific exception object here
                );
                return std::string("(INVALID_VARREF)");
            }

            std::string name = varName->getText();
            if (!IN_VECTOR(usedVarNames, name)) {
                this->parser->notifyErrorListeners(
                    context->getStart(), 
                    "referencing an undefined variable",
                    nullptr // We don't need to pass a specific exception object here
                );
            }
            return name.erase(0, 1);
        }
};

bool parserErrors = false;
class ErrorListener : public BaseErrorListener {
    private:
        // (Storing a std::string value will result in a C4251 MSVC warning)
        const std::string src; // "Lexer" or "Parser"
    public:
        // Constructors
        ErrorListener() = default;
        ErrorListener(const std::string errorSource) : src(errorSource) {}

        // ANTLR4 functions
        void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token *offendingSymbol, size_t line,
            size_t charPositionInLine, const std::string &msg, std::exception_ptr e) override {

            ERR << msg << " ("
                << ((offendingSymbol != nullptr) ? offendingSymbol->getText() : "<N/A>") << ")"
                << "\n    source: " << src << "@" << line << ":" << charPositionInLine 
                << ERR_END;

            parserErrors = true;
        }
        void reportAmbiguity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa, size_t startIndex,
            size_t stopIndex, bool exact, const antlrcpp::BitSet &ambigAlts, antlr4::atn::ATNConfigSet *configs)
            override {

            antlr4::TokenStream *tokens = recognizer->getTokenStream();
            std::string ambiguousText = tokens->getText(antlr4::misc::Interval(startIndex, stopIndex));

            antlr4::Token *startToken = tokens->get(startIndex);
            size_t line = startToken->getLine();
            size_t pos = startToken->getCharPositionInLine();

            ERR << "ambiguity detected:\n"
                << "    \"" << ambiguousText << "\"\n"
                << "\n    source: " << src << "@" << line << ":" << pos
                << ERR_END;

            parserErrors = true;
        }
        void reportAttemptingFullContext(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, const antlrcpp::BitSet &conflictingAlts,
            antlr4::atn::ATNConfigSet *configs) override { }
        void reportContextSensitivity(antlr4::Parser *recognizer, const antlr4::dfa::DFA &dfa,
            size_t startIndex, size_t stopIndex, size_t prediction,
            antlr4::atn::ATNConfigSet *configs) override { }
};

std::string processData(const std::string &path, const std::string &content, int zoneId) {
    // Read content
    ANTLRInputStream input(content);

    GeneratedLexer::DiagnosticsCodeLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    DiagnosticsCodeParser parser(&tokens);

    // Check for errors
    ErrorListener errorListener = ErrorListener(path);
    lexer.removeErrorListeners();// remove default parser error listeners.
    lexer.addErrorListener(&errorListener);
    parser.removeErrorListeners();// remove default parser error listeners.
    parser.addErrorListener(&errorListener);

    // Print
    tokens.fill();
    if (debug) {
        std::cout << "========== TOKENS ==========\n";
        for (auto token : tokens.getTokens()) {
            std::cout << token->toString() << std::endl;
        }
        std::cout << "============================\n\n";
    }

    // Get parse tree
    tree::ParseTree *tree = parser.list();

    // Print
    if (debug) {
        std::cout << "======== PARSE TREE ========\n";
        std::cout << tree->toStringTree(&parser) << std::endl;
        std::cout << "============================\n";
    }

    // Analyse code and insert it into 'output'!
    Visitor visitor = Visitor(&parser, path, zoneId);
    return std::move(any_cast<std::string>(visitor.visit(tree)));
}

bool deleteFile(const std::string &path) {
    if (fs::exists(path)) {
        std::cout << "File already exists. Attempting to delete...\n";
        
        // Step 2: Delete the file
        bool success = fs::remove(path);
        
        if (success) {
            std::cout << "File deleted successfully.\n";
        } else {
            ERR << "Failed to delete the file (check permissions).\n" << ERR_END;
        }
        return success;
    }
    return true;
}

int readFileContents(const std::string& filePath, std::string &output) {
    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        ERR << "Could not open file " << filePath << "\n" << ERR_END;
        return 3;
    }

    std::stringstream buffer;
    buffer << fileStream.rdbuf();

    // Extract data
    output = std::move(buffer.str());
    return 0;
}

int checkFilename(const std::string &name) {
    if (!isDigit(name)) {
        ERR << "All .diag file names must only contain digits! ('" << name << "')\n" << ERR_END;
        return 7;
    }
    return 0;
}

int lookupFiles(const std::string& dir, std::ostringstream &output) {
    std::cout << "Looking up files wihin" << dir <<"\n";

    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        ERR << "Directory does not exist or is invalid: " << dir << "\n" << ERR_END;
        return 2;
    }

    // Skip folders we don't have permission to read!
    auto options = fs::directory_options::skip_permission_denied;

    std::cout << "Scanning directory: " << dir << "\n";
    for (const auto& entry : fs::recursive_directory_iterator(dir, options)) {
        
        // 4. Check if the current entry is an actual file (not a sub-folder or symlink)
        if (entry.is_regular_file(ec)) {

            fs::path path = entry.path();

            if (path.extension().string() == ".diag") {
                std::string rawPath = path.string();

                std::string zone = path.stem().string();
                int nameState = checkFilename(zone);
                if (nameState != 0) {
                    return nameState;
                }

                std::cout << "Processing file: " << rawPath << "\n";

                // Read file content
                std::string content;
                int state = readFileContents(rawPath, content);
                if (state != 0) {
                    return state;
                }

                // Process data!
                output << std::move(processData(rawPath, content, std::stoi(zone)));
            }
        }
    }

    return 0;
}

int main(int argc, const char* argv[]) {
    std::cout << "Generating Diagnostic codes...\n";

    if (argc < 3) {
        ERR << "Must pass at least three arguments! (outputDir, outputFilename, dir, ...)\n" << ERR_END;
        return 1;
    }

    // Check output data
    std::string outputDir(argv[1]);
    std::string outputFilename(argv[2]);
    std::error_code ec;
    if (!fs::exists(outputDir, ec) || !fs::is_directory(outputDir, ec)) {
        ERR << "Directory does not exist or is invalid: " << outputDir << "\n" << ERR_END;
        return 2;
    }
    fs::path dir = outputDir;
    fs::path file = outputFilename;
    fs::path path = dir/file;
    std::string outputPath = path.string();

    // outputPath
    if (!deleteFile(outputPath)) {
        ERR << "Couldn't clean up the output file: " << outputPath << "\n" << ERR_END;
        return 4;
    }

    // Process data!
    std::ostringstream outputData;
    outputData << "// GENERATED DIAGNOSTIC CODES RESOURCE!\n";
    outputData << "#pragma once\n";
    outputData << "#include <iostream>\n";
    outputData << "#include <string>\n";
    outputData << "#include <format>\n";
    outputData << "#if defined(_MSC_VER)\n";
    outputData << "    #define FORCE_INLINE __forceinline\n";
    outputData << "#elif defined(__GNUC__) || defined(__clang__)\n";
    outputData << "    #define FORCE_INLINE __attribute__((always_inline)) inline\n";
    outputData << "#else\n";
    outputData << "    #define FORCE_INLINE inline\n";
    outputData << "#endif\n";

    outputData << "\n";
    int latestFailure = 0;
    for (int i = 3; i < argc; i++) {
        std::string rootDir(argv[i]);
        int state = lookupFiles(rootDir, outputData);
        if (state != 0) {
            latestFailure = state;
        }
    }
    if (parserErrors) {
        return 5;
    }

    // Output data!
    std::ofstream outFile(outputPath);
    // 4. Check if the file opened successfully before writing
    if (outFile.is_open()) {
        // Write the string to the file
        outFile << outputData.str();
    
        // Always close the file when you are done!
        outFile.close(); 
    
        std::cout << "Successfully saved to: " << outputPath << "\n";
    } else {
        // If there's a permissions issue or bad path, this will catch it
        ERR << "Unable to open file: " << outputPath << "\n" << ERR_END;
        return 6;
    }

    return latestFailure;
}
