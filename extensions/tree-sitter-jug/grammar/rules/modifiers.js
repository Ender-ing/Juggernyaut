const KWD = require('../base/keywords');

const withModifiers = ($, ...ruleSeq) => seq(
    optional(field('modifiers', $.modifiers)),
    ...ruleSeq
);

module.exports = {
    withModifiers,

    rules: {
        modifiers: $ => repeat1($._modifier),

        _modifier: $ => choice(
            $.mod_export,
            // $.mod_inline,
            // $.mod_comptime,
        ),

        mod_export: _ => KWD.KWD_EXPORT,
    }
};
