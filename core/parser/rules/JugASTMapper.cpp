/**
 * @brief
 * AST rules & scopes mapper
**/

#include "JugASTMapper.hpp"

namespace Parser::Rules {
    void registerScopes(Visitors::ASTMapper &mapper) {
        mapper.bindNode("prog",
            Visitors::NodeRule{.astKind = Data::Parser::ASTKind::Program});
        mapper.bindNode("generic_block",
            Visitors::NodeRule{.astKind = Data::Parser::ASTKind::Block});
    }

    const Visitors::ASTMapper& getJugASTMapper(ts::Language lang) {
        static const Visitors::ASTMapper instance = [&]() {
            Visitors::ASTMapper m(lang);
            registerScopes(m);
            return m;
        }();
        return instance;
    }
}
