const KWD = require('../base/keywords');
const SYM = require('../base/symbols');

module.exports = {
    rules: {
        // Initial scope in a file (MUST remain the first rule)
        prog: $ => repeat($._global),

        // Globals
        _global: $ => choice(
            $._root_statement,
            seq(optional(KWD.KWD_EXPORT), $._declaration)
        ),

        // Zoned blocks
        generic_block: $ => seq(
            SYM.SYM_CURLY_BRACKET_LEFT,
            repeat($._generic_content),
            SYM.SYM_CURLY_BRACKET_RIGHT
        ),

        _generic_content: $ => choice(
            $._statement,
            $.decl_data,
            $.generic_block
        )
    }
};
