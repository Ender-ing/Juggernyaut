/**
 * @brief
 * Jug statement lowering rules umbrella
**/

#include "statements.hpp"

// Statements
#include "import.hpp"

namespace Parser::Rules {
    void registerStatements(Lowering::LoweringTable &table) {
        Statements::registerImport(table);
        // Statements::registerExport(table);
        // Statements::registerBind(table);
        // Statements::registerControlFlow(table);
    }
}
