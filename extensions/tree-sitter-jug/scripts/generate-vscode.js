const fs = require('fs');
const path = require('path');

// Import up-to-date grammar!
const KWD = require('../grammar/base/keywords');
const SYM = require('../grammar/base/symbols');

const escapeRegex = (str) => str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

// Group keywords from grammar/base/keywords.js
const importKwds = [KWD.KWD_IMPORT, KWD.KWD_EXPORT, KWD.KWD_BIND];
const storageTypes = [KWD.KWD_LET, KWD.KWD_FUNC, KWD.KWD_TYPE, KWD.KWD_ABSTRACT];
const storageModifiers = [KWD.KWD_CONST, KWD.KWD_REF];
const sillyKwds = [KWD.CAT, KWD.KITTY, KWD.MEOW, KWD.BAD_CODE];
const usedKwds = new Set([...importKwds, ...storageTypes, ...storageModifiers, ...sillyKwds]);
const flowKwds = Object.values(KWD).filter((k) => !usedKwds.has(k));

// Sort operators longest-first so multi-char operators match before single-char ones
const allOperators = Object.values(SYM)
    .filter((s) => !['{', '}', '[', ']', '(', ')'].includes(s))
    .sort((a, b) => b.length - a.length)
    .map(escapeRegex);

const tmLanguage = {
    $schema: 'https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json',
    name: 'Juggernyaut',
    scopeName: 'source.juggernyaut',
    patterns: [{ include: '#root-order' }],
    repository: {
        'root-order': {
            patterns: [
                { include: '#comments' },
                { include: '#literals' },
                { include: '#keywords' },
                { include: '#operators' },
            ],
        },
        comments: {
            patterns: [
                {
                    name: 'comment.block.documentation.juggernyaut',
                    begin: '/\\*',
                    end: '\\*/',
                    patterns: [{ include: '#comment-highlights' }],
                },
                {
                    name: 'comment.line.double-slash.juggernyaut',
                    begin: '//',
                    end: '\\n',
                    patterns: [{ include: '#comment-highlights' }],
                },
            ],
        },
        'comment-highlights': {
            patterns: [
                {
                    name: 'comment.highlight.warning.juggernyaut',
                    match: '(?<![^\\s])(@!.+)(?=\\n)',
                    captures: { 1: { name: 'invalid.warning.juggernyaut' } },
                },
                {
                    name: 'comment.highlight.title.juggernyaut',
                    match: '(?<![^\\s])(@[^\\s]+)',
                    captures: { 1: { name: 'markup.inserted.juggernyaut' } },
                },
                {
                    name: 'comment.highlight.escape.juggernyaut',
                    begin: '{(?=[^\\n]*\\}[^\\n]*)',
                    end: '}',
                    captures: { 0: { name: 'markup.italic.juggernyaut' } },
                    patterns: [
                        { include: '#root-order' },
                        { name: 'markup.italic.juggernyaut', match: '[^\\n]+?' },
                    ],
                },
            ],
        },
        literals: {
            patterns: [
                { include: '#strings' },
                { include: '#chars' },
                { include: '#booleans' },
                { include: '#numbers' },
            ],
        },
        strings: {
            patterns: [
                {
                    name: 'string.quoted.double.juggernyaut',
                    begin: '"',
                    end: '(((?<!\\\\)")|((?<!\\\\)\\r?\\n))',
                    patterns: [{ include: '#escapeSequences' }],
                },
            ],
        },
        chars: {
            patterns: [
                {
                    name: 'string.quoted.single.juggernyaut',
                    begin: "'",
                    end: "(((?<!\\\\)')|((?<!\\\\)\\r?\\n))",
                    patterns: [{ include: '#escapeSequences' }],
                },
            ],
        },
        escapeSequences: {
            patterns: [
                {
                    name: 'constant.character.escape.juggernyaut',
                    match: '\\\\([nrt0"\'bfv\\\\]|\\r?\\n)',
                },
                {
                    name: 'invalid.illegal.juggernyaut',
                    match: '\\\\([^nrt0"\'bfv\\\\\\r\\n])',
                },
            ],
        },
        booleans: {
            patterns: [
                {
                    name: 'constant.language.boolean.juggernyaut',
                    match: '\\b(true|false)\\b',
                },
            ],
        },
        numbers: {
            patterns: [
                {
                    name: 'constant.numeric.juggernyaut',
                    match: '\\b(0[bB][01]+|0[oO][0-7]+|0[xX][0-9a-fA-F]+|([1-9]|[12][0-9]|3[0-6])d[0-9a-zA-Z]+|[0-9]*\\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+)\\b',
                },
            ],
        },
        keywords: {
            patterns: [
                {
                    name: 'keyword.control.import.juggernyaut',
                    match: `\\b(${importKwds.join('|')})\\b`,
                },
                {
                    name: 'storage.type.juggernyaut',
                    match: `\\b(${storageTypes.join('|')})\\b`,
                },
                {
                    name: 'storage.modifier.juggernyaut',
                    match: `\\b(${storageModifiers.join('|')})\\b`,
                },
                {
                    name: 'keyword.control.logic.juggernyaut',
                    match: `\\b(${flowKwds.join('|')})\\b`,
                },
                {
                    name: 'keyword.control.silly.juggernyaut',
                    match: `\\b(${sillyKwds.join('|')})\\b`,
                },
            ],
        },
        operators: {
            patterns: [
                {
                    name: 'keyword.operator.juggernyaut',
                    match: `(${allOperators.join('|')})`,
                },
            ],
        },
    },
};

// Write grammar updates
const tmPath = path.resolve(__dirname, '../../VSCode/syntaxes/juggernyaut.tmLanguage.json');
fs.writeFileSync(tmPath, JSON.stringify(tmLanguage, null, 2) + '\n');
console.log('Updated VSCode syntax:', tmPath);
