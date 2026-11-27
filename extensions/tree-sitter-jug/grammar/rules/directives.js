const SYM = require('../base/symbols');

module.exports = {
    rules: {
        // @name or @name(arg1, arg2, ...)
        directive: $ => prec.right(seq(
            SYM.SYM_AT,
            field('name', $.identifier),
            optional(seq(
                SYM.SYM_PARENTHESES_LEFT,
                optional(field('args', $.expr_list)),
                SYM.SYM_PARENTHESES_RIGHT
            ))
        ))
    }
};
