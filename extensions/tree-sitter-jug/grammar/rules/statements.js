const KWD = require('../base/keywords');
const SYM = require('../base/symbols');

module.exports = {
    rules: {
        // Statement categories
        _statement: $ => choice(
            $._stat_empty,
            $.stat_expr,
            $.stat_return,
            $.stat_end,
            $.stat_skip,
            $.stat_fall,
            $.stat_if,
            $.stat_iteration,
            $.stat_switch,
            $.stat_silly
        ),

        _root_statement: $ => choice(
            $._stat_empty,
            $.stat_import,
            $.stat_export,
            $.stat_bind,
            $.stat_silly
        ),

        // Import statement
        stat_import: $ => choice(
            $.import_discrete,
            $.import_encapsulate
        ),

        import_discrete: $ => seq(
            KWD.KWD_IMPORT,
            SYM.SYM_CURLY_BRACKET_LEFT,
            optional(field('list', $.import_list)),
            SYM.SYM_CURLY_BRACKET_RIGHT,
            KWD.KWD_FROM,
            field('source', $.import_source),
            SYM.SYM_SEMICOLON
        ),

        import_encapsulate: $ => seq(
            KWD.KWD_IMPORT,
            field('source', $.import_source),
            KWD.KWD_AS,
            field('handle', $.identifier),
            SYM.SYM_SEMICOLON
        ),

        import_list: $ => seq(
            $.identifier,
            repeat(seq(SYM.SYM_COMMA, $.identifier))
        ),

        import_source: $ => choice(
            $.import_library,
            $.import_path
        ),

        import_path: $ => $.lit_string,

        import_library: $ => seq(
            $.identifier,
            optional(seq(SYM.SYM_DOT, $.identifier))
        ),

        // Export statement
        stat_export: $ => $.export_discrete,

        export_discrete: $ => seq(
            KWD.KWD_EXPORT,
            field('items', $.identifier),
            repeat(seq(SYM.SYM_COMMA, field('items', $.identifier))),
            SYM.SYM_SEMICOLON
        ),

        // Bind statement
        stat_bind: $ => seq(
            optional(KWD.KWD_EXPORT),
            KWD.KWD_BIND,
            optional(field('bindMode', $.lit_string)),
            choice($.bind_item, $.bind_block)
        ),

        bind_block: $ => seq(
            SYM.SYM_CURLY_BRACKET_LEFT,
            repeat($.bind_item),
            SYM.SYM_CURLY_BRACKET_RIGHT
        ),

        bind_item: $ => seq(
            choice(
                field('funcDetails', $.function_details),
                seq(field('dataState', $.data_storage), field('dataDetails', $.data_details))
            ),
            optional(seq(
                KWD.KWD_FROM,
                field('originalName', $.lit_string),
                choice(
                    seq(
                        SYM.SYM_PARENTHESES_LEFT,
                        repeat($.data_details),
                        SYM.SYM_PARENTHESES_RIGHT,
                        SYM.SYM_ARROW_RIGHT,
                        field('returnType', $.type_spec)
                    ),
                    seq(SYM.SYM_COLON, field('type', $.type_spec))
                ),
            )),
            SYM.SYM_SEMICOLON
        ),

        // Functional / loop jump statements
        stat_return: $ => seq(
            KWD.KWD_RETURN,
            optional(field('value', $._expr)),
            SYM.SYM_SEMICOLON
        ),

        stat_end: _ => seq(KWD.KWD_END, SYM.SYM_SEMICOLON),
        stat_skip: _ => seq(KWD.KWD_SKIP, SYM.SYM_SEMICOLON),
        stat_fall: _ => seq(KWD.KWD_FALL, SYM.SYM_SEMICOLON),

        // Conditional & loop statements
        stat_if: $ => seq(
            KWD.KWD_IF,
            SYM.SYM_PARENTHESES_LEFT,
            field('condition', $._expr),
            SYM.SYM_PARENTHESES_RIGHT,
            field('body', $.generic_block),
            optional(seq(
                KWD.KWD_ELSE,
                choice(
                    field('elseBody', $.generic_block),
                    field('chainedFlow', $.stat_if)
                ),
            )),
        ),

        stat_iteration: $ => choice(
            seq(
                KWD.KWD_WHILE,
                SYM.SYM_PARENTHESES_LEFT,
                field('condition', $._expr),
                SYM.SYM_PARENTHESES_RIGHT,
                field('body', $.generic_block)
            ),
            seq(
                KWD.KWD_DO,
                field('body', $.generic_block),
                KWD.KWD_WHILE,
                SYM.SYM_PARENTHESES_LEFT,
                field('condition', $._expr),
                SYM.SYM_PARENTHESES_RIGHT,
                SYM.SYM_SEMICOLON
            ),
            seq(
                KWD.KWD_FOR,
                SYM.SYM_PARENTHESES_LEFT,
                field('initState', $.data_storage),
                field('init', $.data_item),
                SYM.SYM_SEMICOLON,
                field('condition', $._expr),
                SYM.SYM_SEMICOLON,
                field('action', $._expr),
                SYM.SYM_PARENTHESES_RIGHT,
                field('body', $.generic_block)
            ),
            seq(
                KWD.KWD_FOR,
                SYM.SYM_PARENTHESES_LEFT,
                field('itemStorage', $.data_storage),
                field('item', $.data_details),
                KWD.KWD_IN,
                field('iterable', $.identifier),
                SYM.SYM_PARENTHESES_RIGHT,
                field('body', $.generic_block)
            ),
        ),

        stat_switch: $ => seq(
            KWD.KWD_SWITCH,
            SYM.SYM_PARENTHESES_LEFT,
            field('target', $._expr),
            SYM.SYM_PARENTHESES_RIGHT,
            SYM.SYM_CURLY_BRACKET_LEFT,
            repeat($.switch_arm),
            SYM.SYM_CURLY_BRACKET_RIGHT
        ),

        switch_arm: $ => choice(
            seq(KWD.KWD_CASE, field('patterns', $.expr_list), field('body', $.generic_block)),
            seq(KWD.KWD_ELSE, field('body', $.generic_block))
        ),

        // Expression, empty & silly statements
        stat_expr: $ => seq($._expr, SYM.SYM_SEMICOLON),
        _stat_empty: _ => SYM.SYM_SEMICOLON,

        stat_silly: _ => seq(
            choice(KWD.CAT, KWD.KITTY, KWD.MEOW, KWD.BAD_CODE),
            SYM.SYM_SEMICOLON
        )
    }
};
