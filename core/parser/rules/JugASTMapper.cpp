/**
 * @brief
 * Jug language CST-to-AST lowering rules
**/

#include "JugASTMapper.hpp"

namespace Parser::Rules {
    void registerScopes(Lowering::LoweringTable &table) {
        table.bindNode("prog", Data::Parser::ASTKind::Program);
        table.bindNode("generic_block", Data::Parser::ASTKind::Block);
    }

    const Lowering::LoweringTable& getJugLoweringTable(ts::Language lang) {
        static const Lowering::LoweringTable instance = [&]() {
            Lowering::LoweringTable t(lang);
            registerScopes(t);
            //registerStatements(t);
            return t;
        }();
        return instance;
    }
}
