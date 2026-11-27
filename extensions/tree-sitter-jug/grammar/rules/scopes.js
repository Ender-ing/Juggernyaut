const KWD = require('../base/keywords');
const SYM = require('../base/symbols');

module.exports = {
    rules: {
        // Initial scope in a file (MUST remain the first rule)
        prog: $ => repeat($._global),

        // Globals
        _global: $ => seq(
            optional(field('directives', repeat1($.directive))),
            choice($._root_statement, $._declaration)
        ),

        // Zoned blocks
        generic_block: $ => seq(
            SYM.SYM_CURLY_BRACKET_LEFT,
            repeat($._generic_content),
            SYM.SYM_CURLY_BRACKET_RIGHT
        ),

        _generic_content: $ => seq(
            optional(field('directives', repeat1($.directive))),
            choice(
                $._statement,
                $.decl_data,
                $.generic_block
            )
        )
    }
};
