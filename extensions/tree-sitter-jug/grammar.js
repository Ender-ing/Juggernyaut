/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const base = require('./grammar/base');
const rules = require('./grammar/rules');

module.exports = grammar({
    name: 'jug',

    extras: base.extras,

    word: $ => $.identifier,

    supertypes: rules.supertypes,

    rules: {
        ...rules.rules,
        ...base.rules,
    }
});
