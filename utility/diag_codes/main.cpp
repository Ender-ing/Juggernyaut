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

// ANTLR runtime
#include "antlr4-runtime.h"
#include "DiagnosticsCodeLexer.h"
#include "DiagnosticsCodeParser.h"

using namespace antlr4;
using namespace GeneratedParser;
namespace fs = std::filesystem;

int main(int argc, const char* argv[]) {
    // 1. Setup the input. Using a hardcoded string to test your error code syntax quickly.
    // You can swap this with ANTLRFileStream to read from a file instead.
    std::string testInput = "0[1, \"test\"]:\n-Oh noooo!\n";
    ANTLRInputStream input(testInput);

int processData(const std::string &path, const std::string &content) {
    // Read content
    ANTLRInputStream input(content);

    // Generate a token stream
    GeneratedLexer::DiagnosticsCodeLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Print
    tokens.fill();
    std::cout << "========== TOKENS ==========\n";
    for (auto token : tokens.getTokens()) {
        std::cout << token->toString() << std::endl;
    }
    std::cout << "============================\n\n";

    // Get parse tree
    GeneratedParser::DiagnosticsCodeParser parser(&tokens);
    tree::ParseTree *tree = parser.list();

    // Print
    std::cout << "======== PARSE TREE ========\n";
    std::cout << tree->toStringTree(&parser) << std::endl;
    std::cout << "============================\n";

    return 0;
}

bool deleteFile(const std::string &path) {
    if (fs::exists(path)) {
        std::cout << "File already exists. Attempting to delete...\n";
        
        // Step 2: Delete the file
        bool success = fs::remove(path);
        
        if (success) {
            std::cout << "File deleted successfully.\n";
        } else {
            ERR << "Failed to delete the file (check permissions).\n";
        }
        return success;
    }
    return true;
}

int readFileContents(const std::string& filePath, std::string &output) {
    // 1. Open the file as an input stream
    std::ifstream fileStream(filePath);

    // 2. Always check if the file actually opened successfully!
    // It could be locked by another program or have restrictive permissions.
    if (!fileStream.is_open()) {
        ERR << "Could not open file " << filePath << "\n";
        return 3;
    }

    // 3. Create a stringstream buffer
    std::stringstream buffer;

    // 4. Dump the entire file stream buffer into the string stream
    buffer << fileStream.rdbuf();

    // 5. Extract and return the final string
    output = std::move(buffer.str());
    return 0;
}

int lookupFiles(const std::string& dir) {
    std::cout << "Looking up files wihin" << dir <<"\n";

    // 1. Verify the starting directory actually exists
    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        ERR << "Directory does not exist or is invalid: " << dir << "\n";
        return 2;
    }

    // 2. Set options to gracefully skip folders we don't have permission to read.
    // Without this, the program will throw an exception and crash on system folders.
    auto options = fs::directory_options::skip_permission_denied;

    std::cout << "Scanning directory: " << dir << "\n";

    // 3. Set up the recursive iterator
    for (const auto& entry : fs::recursive_directory_iterator(dir, options)) {
        
        // 4. Check if the current entry is an actual file (not a sub-folder or symlink)
        if (entry.is_regular_file(ec)) {

            fs::path path = entry.path();

            if (path.filename().string() == ".diag") {
                std::string rawPath = path.string();

                std::cout << "Processing file: " << rawPath << "\n";

                // Read file content
                std::string content;
                int state = readFileContents(rawPath, content);
                if (state != 0) {
                    return state;
                }

                // Process data!
                processData(rawPath, content);
            }
        }
    }

    return 0;
}

int main(int argc, const char* argv[]) {
    std::cout << "Generating Diagnostic codes...\n";

    if (argc < 3) {
        ERR << "Must pass at least three arguments! (outputDir, outputFilename, dir, ...)\n";
        return 1;
    }

    // Check output data
    std::string outputDir(argv[1]);
    std::string outputFilename(argv[2]);
    std::error_code ec;
    if (!fs::exists(outputDir, ec) || !fs::is_directory(outputDir, ec)) {
        ERR << "Directory does not exist or is invalid: " << outputDir << "\n";
        return 2;
    }
    fs::path dir = outputDir;
    fs::path file = outputFilename;
    fs::path path = dir/file;
    std::string outputPath = path.string();

    // outputPath
    if (!deleteFile(outputPath)) {
        ERR << "Couldn't clean up the output file: " << outputPath << "\n";
        return 4;
    }

    // Process data!
    int latestFailure = 0;
    for (int i = 3; i < argc; i++) {
        std::string rootDir(argv[i]);
        int state = lookupFiles(rootDir);
        if (state != 0) {
            latestFailure = state;
        }
    }

    return latestFailure;
}
