const ignores = require('./ignores');
const keywords = require('./keywords');
const symbols = require('./symbols');
const literals = require('./literals');
const identifiers = require('./identifiers');

module.exports = {
    extras: ignores.extras,
    keywords,
    symbols,
    rules: {
        ...literals.rules,
        ...identifiers.rules,
        ...ignores.rules
    }
};
