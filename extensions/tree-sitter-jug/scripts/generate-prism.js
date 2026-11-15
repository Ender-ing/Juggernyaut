const fs = require('fs');
const path = require('path');

// Import up-to-date grammar!
const KWD = require('../grammar/base/keywords');
const SYM = require('../grammar/base/symbols');

const escapeRegex = (str) => str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

const allKeywords = Object.values(KWD);

// Sort operators longest-first so multi-char operators match before single-char ones
const allOperators = Object.values(SYM)
    .filter((s) => !['{', '}', '[', ']', '(', ')'].includes(s))
    .sort((a, b) => b.length - a.length)
    .map(escapeRegex);

const prismSource = `// Auto-generated from extensions/tree-sitter-jug/grammar/base
Prism.languages.juggernyaut = {
    // Comments
    'comment': [
        {
            pattern: /\\/\\*([\\s\\S]*?)\\*\\//,
            greedy: true
        },
        {
            pattern: /\\/\\/.*/,
            greedy: true
        }
    ],

    // Literals
    'string': {
        pattern: /(["'])(?:\\\\(?:\\r\\n|[\\s\\S])|(?!\\1)[^\\\\\\r\\n])*(\\1|\\n)/,
        greedy: true
    },
    'boolean': /\\b(?:true|false)\\b/,
    'number': /\\b(?:0[bB][01]+|0[oO][0-7]+|0[xX][0-9a-fA-F]+|(?:[1-9]|[12][0-9]|3[0-6])d[0-9a-zA-Z]+|[0-9]*\\.[0-9]+(?:[eE][+-]?[0-9]+)?|[0-9]+)\\b/,

    // Keywords
    'keyword': /\\b(?:${allKeywords.join('|')})\\b/,

    // Operators & Punctuation
    'operator': /${allOperators.join('|')}/,
    'punctuation': /[{}[\\]();,.:]/
};
Prism.languages.jug = Prism.languages.juggernyaut;
`;

// Write grammar updates
const prismPath = path.resolve(__dirname, '../../prism.juggernyaut.js');
fs.writeFileSync(prismPath, prismSource);
console.log('Updated Prism syntax:', prismPath);
