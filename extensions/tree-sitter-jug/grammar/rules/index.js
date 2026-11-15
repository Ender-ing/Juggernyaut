const scopes = require('./scopes');
const statements = require('./statements');
const declarations = require('./declarations');
const expressions = require('./expressions');
const operations = require('./operations');
const types = require('./types');

module.exports = {
    PREC: expressions.PREC,

    supertypes: $ => [
        $._expr,
        $._statement,
        $._root_statement,
        $._declaration,
        $._lit
    ],

    rules: {
        ...scopes.rules, // Must be first so `prog` is the start rule!
        ...statements.rules,
        ...declarations.rules,
        ...types.rules,
        ...expressions.rules,
        ...operations.rules
    }
};
