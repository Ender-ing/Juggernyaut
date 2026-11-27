const SYM = require('../base/symbols');

const PREC = {
    ASSIGN: 1,
    LOGICAL: 2,
    COMPARISON: 3,
    PRIORITY_LOW: 4,
    ADDITIVE: 5,
    MULTIPLICATIVE: 6,
    PRIORITY_HIGH: 7,
    PREFIX: 8,
    POSTFIX: 9,
    INDEX: 10,
    CALL: 11,
    MEMBER: 12
};

module.exports = {
    PREC,

    rules: {
        // General expression supertype
        _expr: $ => choice(
            $.expr_grouped,
            $.expr_member,
            $.expr_call,
            $.expr_index,
            $.expr_postfix,
            $.expr_prefix,
            $.expr_priority_high,
            $.expr_multiplicative,
            $.expr_additive,
            $.expr_priority_low,
            $.expr_comparison,
            $.expr_logical,
            $.expr_assign,
            $.identifier,
            $._lit
        ),

        // Grouping & access expressions
        expr_grouped: $ => seq(
            SYM.SYM_PARENTHESES_LEFT,
            $._expr,
            SYM.SYM_PARENTHESES_RIGHT
        ),

        expr_member: $ => prec.left(PREC.MEMBER, seq(
            field('target', $._expr),
            SYM.SYM_DOT,
            field('member', $.identifier)
        )),

        expr_call: $ => prec.left(PREC.CALL, seq(
            field('callee', $._expr),
            SYM.SYM_PARENTHESES_LEFT,
            optional($.expr_list),
            SYM.SYM_PARENTHESES_RIGHT
        )),

        expr_index: $ => prec.left(PREC.INDEX, seq(
            field('target', $._expr),
            SYM.SYM_SQUARE_BRACKET_LEFT,
            field('indices', $.expr_list),
            SYM.SYM_SQUARE_BRACKET_RIGHT
        )),

        // Unary math / bitwise expressions
        expr_postfix: $ => prec.left(PREC.POSTFIX, seq(
            field('operand', $._expr),
            field('op', $.op_postfix)
        )),

        expr_prefix: $ => prec.right(PREC.PREFIX, seq(
            field('op', $.op_prefix),
            field('operand', $._expr)
        )),

        // Binary math / bitwise / range expressions
        expr_priority_high: $ => prec.right(PREC.PRIORITY_HIGH, seq(
            field('left', $._expr),
            field('op', $.op_priority_high),
            field('right', $._expr)
        )),

        expr_multiplicative: $ => prec.left(PREC.MULTIPLICATIVE, seq(
            field('left', $._expr),
            field('op', $.op_multiplicative),
            field('right', $._expr)
        )),

        expr_additive: $ => prec.left(PREC.ADDITIVE, seq(
            field('left', $._expr),
            field('op', $.op_additive),
            field('right', $._expr)
        )),

        expr_priority_low: $ => prec.left(PREC.PRIORITY_LOW, seq(
            field('left', $._expr),
            field('op', $.op_priority_low),
            field('right', $._expr)
        )),

        // Relational, logical & assignment expressions
        expr_comparison: $ => prec.left(PREC.COMPARISON, seq(
            field('left', $._expr),
            field('op', $.op_comparison),
            field('right', $._expr)
        )),

        expr_logical: $ => prec.left(PREC.LOGICAL, seq(
            field('left', $._expr),
            field('op', $.op_logical),
            field('right', $._expr)
        )),

        expr_assign: $ => prec.right(PREC.ASSIGN, seq(
            field('left', $._expr),
            field('op', $.op_assign),
            field('right', $._expr)
        )),

        // Comma-separated expression list
        expr_list: $ => seq(
            $._expr,
            repeat(seq(SYM.SYM_COMMA, $._expr))
        )
    }
};
