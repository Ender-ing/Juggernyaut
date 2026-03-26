#include <iostream>
#include <string>
#include <vector>

// 1. Trigger: performance-unnecessary-value-param
// We are passing a large string by value instead of const std::string&
void processWord(std::string word) {
    
    // 2. Trigger: performance-unnecessary-copy-initialization
    // We are making a copy of the string but only ever reading from it
    std::string wordCopy = word; 
    
    std::cout << "Processing: " << wordCopy << "\n";
}

int main() {
    std::vector<std::string> dictionary = {"valgrind", "clang-tidy", "memory", "leak"};

    // 3. Trigger: performance-for-range-copy
    // We are copying each string out of the vector instead of using const auto&
    for (std::string item : dictionary) {
        processWord(item);
    }

    return 0;
}
