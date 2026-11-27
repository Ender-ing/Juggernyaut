/**
 * @brief
 * Jug language CST-to-AST lowering rules
**/

#include "JugASTMapper.hpp"

// C++ headers
#include <cassert>

// Base rules
#include "base/identifiers.hpp"
#include "base/literals.hpp"

namespace Parser::Rules {
    void registerBase(Lowering::LoweringTable &table) {
        Base::registerIdentifiers(table);
        Base::registerLiterals(table);
    }

    const Lowering::LoweringTable& getJugLoweringTable(ts::Language lang) {
        // Read-only shared instance
        static const Lowering::LoweringTable instance = [&]() {
            Lowering::LoweringTable t(lang);
            registerBase(t);
            registerScopes(t);
            registerStatements(t);

            assert(!t.hasRegistrationIssues());
            return t;
        }();
        return instance;
    }
}
