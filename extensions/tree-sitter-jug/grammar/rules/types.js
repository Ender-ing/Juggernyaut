const KWD = require('../base/keywords');
const SYM = require('../base/symbols');

module.exports = {
    rules: {
        // Type declarations
        decl_type: $ => choice(
            $.type_alias,
            $.abstract_alias
        ),

        type_alias: $ => seq(
            KWD.KWD_TYPE,
            field('name', $.identifier),
            SYM.SYM_EQUAL,
            field('value', $._expr),
            SYM.SYM_SEMICOLON
        ),

        abstract_alias: $ => seq(
            KWD.KWD_ABSTRACT,
            field('name', $.identifier),
            SYM.SYM_EQUAL,
            field('value', $._expr),
            SYM.SYM_SEMICOLON
        ),

        // Type specifications
        type_spec: $ => seq(
            field('base', $.identifier),
            optional(seq(
                SYM.SYM_LESS,
                choice(field('internalTypes', $.type_spec), field('internalConsts', $._lit)),
                repeat(seq(
                    SYM.SYM_COMMA,
                    choice(field('internalTypes', $.type_spec), field('internalConsts', $._lit))
                )),
                SYM.SYM_GREATER,
            )),
            repeat(seq(
                SYM.SYM_SQUARE_BRACKET_LEFT,
                optional(field('dims', $.lit_number)),
                SYM.SYM_SQUARE_BRACKET_RIGHT
            ))
        )
    }
};
