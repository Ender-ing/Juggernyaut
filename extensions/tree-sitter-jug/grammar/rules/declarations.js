const KWD = require('../base/keywords');
const SYM = require('../base/symbols');

module.exports = {
    rules: {
        _declaration: $ => choice(
            $.decl_data,
            $.decl_function,
            $.decl_type
        ),

        // Data declaration
        decl_data: $ => seq(
            field('state', $.data_storage),
            $.data_item,
            repeat(seq(SYM.SYM_COMMA, $.data_item)),
            SYM.SYM_SEMICOLON
        ),

        data_item: $ => seq(
            field('details', $.data_details),
            optional(seq(SYM.SYM_EQUAL, field('value', $._expr)))
        ),

        data_details: $ => seq(
            optional(field('name', $.identifier)),
            SYM.SYM_COLON,
            field('type', $.type_spec)
        ),

        data_storage: _ => choice(KWD.KWD_LET, KWD.KWD_CONST, KWD.KWD_REF),

        // Function declaration
        decl_function: $ => seq(
            field('details', $.function_details),
            field('body', $.generic_block)
        ),

        function_details: $ => seq(
            KWD.KWD_FUNC,
            field('name', $.identifier),
            SYM.SYM_PARENTHESES_LEFT,
            optional(seq($.function_argument, repeat(seq(SYM.SYM_COMMA, $.function_argument)))),
            SYM.SYM_PARENTHESES_RIGHT,
            optional(seq(
                SYM.SYM_ARROW_RIGHT,
                optional(field('returnState', $.argument_state)),
                field('returnType', $.type_spec)
            ))
        ),

        function_argument: $ => seq(
            optional(field('state', $.argument_state)),
            field('details', $.data_details),
            optional(seq(SYM.SYM_EQUAL, field('defaultValue', $._expr)))
        ),

        argument_state: _ => choice(KWD.KWD_CONST, KWD.KWD_REF),
    }
};
