/**
 * doruk-lang.js
 * Monaco Editor dil tanımı — DORUK programlama dili
 * Sözdizimi renklendirme, otomatik tamamlama, kod parçacıkları
 */

'use strict';

// Bu dosya Monaco yüklendikten sonra app.js tarafından çağrılır.
// window.registerDorukLanguage() fonksiyonunu dışarı açar.

window.registerDorukLanguage = function (monaco) {

  // ─── Language registration ─────────────────────────────────────────────────
  monaco.languages.register({
    id:         'doruk',
    extensions: ['.drk'],
    aliases:    ['DORUK', 'Doruk', 'drk'],
    mimetypes:  ['text/x-doruk'],
  });

  // ─── Tokenizer (Monarch) ───────────────────────────────────────────────────
  monaco.languages.setMonarchTokensProvider('doruk', {

    keywords: [
      'değişken', 'sabit', 'fonksiyon', 'döndür',
      'sınıf', 'kalıtım', 'yeni', 'bu',
      'eğer', 'değilse', 'döngü', 'için', 'içinde',
      'kır', 'devam',
      've', 'veya', 'değil',
      // ASCII fallbacks (parser may accept both)
      'degisken', 'sabit', 'fonksiyon', 'dondur',
      'sinif', 'kalitim', 'yeni', 'bu',
      'eger', 'degilse', 'dongu', 'icin', 'icinde',
      'kir', 'devam',
      've', 'veya', 'degil',
    ],

    literals: ['doğru', 'yanlış', 'boş', 'dogru', 'yanlis', 'bos'],

    builtins: [
      'yaz', 'yazln', 'oku', 'hata',
      'uzunluk', 'tip',
      'tamSayı', 'ondalık', 'metin', 'liste',
      'ekle', 'sil', 'ters', 'sırala',
      'rastgele', 'zamanDamgası',
      // ASCII variants
      'tamSayi', 'ondalik', 'sirala', 'zamanDamgasi',
    ],

    operators: [
      '=', '!=', '<', '>', '<=', '>=',
      '+', '-', '*', '/', '%',
      '&&', '||', '!',
      '+=', '-=', '*=', '/=',
    ],

    symbols: /[=><!~?:&|+\-*\/\^%]+/,

    escapes: /\\(?:[nrtbf\\'"\/]|u[0-9A-Fa-f]{4})/,

    tokenizer: {
      root: [
        // Whitespace
        [/\s+/, 'white'],

        // Single-line comment
        [/\/\/.*$/, 'comment'],

        // Block comment
        [/\/\*/, { token: 'comment', next: '@block_comment' }],

        // Strings (double quote)
        [/"/, { token: 'string.quote', bracket: '@open', next: '@string_dq' }],

        // Strings (single quote)
        [/'/, { token: 'string.quote', bracket: '@open', next: '@string_sq' }],

        // Numbers
        [/\d+\.\d+([eE][-+]?\d+)?/, 'number.float'],
        [/\d+/, 'number'],

        // Identifiers and keywords
        [/[a-zA-ZğüşıöçĞÜŞİÖÇ_][a-zA-ZğüşıöçĞÜŞİÖÇ0-9_]*/, {
          cases: {
            '@keywords': 'keyword',
            '@literals': 'constant.language',
            '@builtins': 'support.function',
            '@default': 'identifier',
          },
        }],

        // Delimiters & operators
        [/[{}]/, 'delimiter.curly'],
        [/[[\]]/, 'delimiter.bracket'],
        [/[()]/, 'delimiter.parenthesis'],
        [/[;,.]/, 'delimiter'],
        [/@symbols/, {
          cases: {
            '@operators': 'operator',
            '@default': 'operator',
          },
        }],
      ],

      block_comment: [
        [/[^/*]+/, 'comment'],
        [/\*\//, { token: 'comment', next: '@pop' }],
        [/[/*]/, 'comment'],
      ],

      string_dq: [
        [/[^\\"]+/, 'string'],
        [/@escapes/, 'string.escape'],
        [/\\./, 'string.escape.invalid'],
        [/"/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
      ],

      string_sq: [
        [/[^\\']+/, 'string'],
        [/@escapes/, 'string.escape'],
        [/\\./, 'string.escape.invalid'],
        [/'/, { token: 'string.quote', bracket: '@close', next: '@pop' }],
      ],
    },
  });

  // ─── Language configuration ────────────────────────────────────────────────
  monaco.languages.setLanguageConfiguration('doruk', {
    comments: {
      lineComment:  '//',
      blockComment: ['/*', '*/'],
    },
    brackets: [
      ['{', '}'],
      ['[', ']'],
      ['(', ')'],
    ],
    autoClosingPairs: [
      { open: '{',  close: '}' },
      { open: '[',  close: ']' },
      { open: '(',  close: ')' },
      { open: '"',  close: '"',  notIn: ['string'] },
      { open: "'",  close: "'",  notIn: ['string'] },
    ],
    surroundingPairs: [
      { open: '{',  close: '}' },
      { open: '[',  close: ']' },
      { open: '(',  close: ')' },
      { open: '"',  close: '"' },
      { open: "'",  close: "'" },
    ],
    folding: {
      markers: {
        start: /^\s*\/\/\s*#?region\b/,
        end:   /^\s*\/\/\s*#?endregion\b/,
      },
    },
    indentationRules: {
      increaseIndentPattern: /^.*\{[^}"']*$/,
      decreaseIndentPattern: /^\s*\}/,
    },
    onEnterRules: [
      {
        beforeText: /^\s*\/\*\*(?!\/)([^*]|\*(?!\/))*$/,
        action:     { indentAction: 0, appendText: ' * ' },
      },
    ],
  });

  // ─── Completions ───────────────────────────────────────────────────────────
  monaco.languages.registerCompletionItemProvider('doruk', {
    triggerCharacters: ['.', '('],

    provideCompletionItems(model, position) {
      const word  = model.getWordUntilPosition(position);
      const range = {
        startLineNumber: position.lineNumber,
        endLineNumber:   position.lineNumber,
        startColumn:     word.startColumn,
        endColumn:       word.endColumn,
      };

      const KW  = monaco.languages.CompletionItemKind.Keyword;
      const FN  = monaco.languages.CompletionItemKind.Function;
      const SN  = monaco.languages.CompletionItemKind.Snippet;
      const RULE = monaco.languages.CompletionItemInsertTextRule.InsertAsSnippet;

      const keywords = [
        'değişken','sabit','fonksiyon','döndür','sınıf','kalıtım',
        'eğer','değilse','döngü','için','içinde','kır','devam',
        'yeni','bu','ve','veya','değil','doğru','yanlış','boş',
      ].map(kw => ({ label: kw, kind: KW, insertText: kw, range }));

      const builtins = [
        { label: 'yaz',        detail: 'yaz(değer)     — satır sonu yok' },
        { label: 'yazln',      detail: 'yazln(değer)   — satır sonu ile yaz' },
        { label: 'oku',        detail: 'oku()           — kullanıcıdan girdi al' },
        { label: 'hata',       detail: 'hata(mesaj)    — hata fırlat' },
        { label: 'uzunluk',    detail: 'uzunluk(x)     — liste/metin/sözlük boyutu' },
        { label: 'tip',        detail: 'tip(x)         — değer tipi (metin)' },
        { label: 'tamSayı',    detail: 'tamSayı(x)     — tam sayıya çevir' },
        { label: 'ondalık',    detail: 'ondalık(x)     — ondalığa çevir' },
        { label: 'metin',      detail: 'metin(x)       — metne çevir' },
        { label: 'liste',      detail: 'liste(x)       — listeye çevir' },
        { label: 'ekle',       detail: 'ekle(liste, x) — listeye eleman ekle' },
        { label: 'sil',        detail: 'sil(liste, i)  — indisteki elemanı sil' },
        { label: 'ters',       detail: 'ters(liste)    — listeyi ters çevir' },
        { label: 'sırala',     detail: 'sırala(liste)  — listeyi sırala' },
        { label: 'rastgele',   detail: 'rastgele()     — 0-1 arası rastgele sayı' },
        { label: 'zamanDamgası', detail: 'zamanDamgası() — unix zaman damgası' },
      ].map(b => ({
        label:      b.label,
        kind:       FN,
        insertText: b.label,
        detail:     b.detail,
        range,
      }));

      const snippets = [
        {
          label:           'fonksiyon',
          documentation:   'Fonksiyon tanımı',
          insertText:      'fonksiyon ${1:isim}(${2:parametreler}) {\n\t${3:// gövde}\n}',
        },
        {
          label:           'eğer',
          documentation:   'Koşul ifadesi',
          insertText:      'eğer (${1:koşul}) {\n\t${2}\n}',
        },
        {
          label:           'eğer-değilse',
          documentation:   'Koşul-değilse',
          insertText:      'eğer (${1:koşul}) {\n\t${2}\n} değilse {\n\t${3}\n}',
        },
        {
          label:           'döngü',
          documentation:   'Koşullu döngü',
          insertText:      'döngü (${1:koşul}) {\n\t${2}\n}',
        },
        {
          label:           'için',
          documentation:   'For döngüsü',
          insertText:      'için (${1:i} = ${2:0}; ${1:i} < ${3:10}; ${1:i} = ${1:i} + 1) {\n\t${4}\n}',
        },
        {
          label:           'için-içinde',
          documentation:   'For-each döngüsü',
          insertText:      'için (${1:eleman} içinde ${2:liste}) {\n\t${3}\n}',
        },
        {
          label:           'sınıf',
          documentation:   'Sınıf tanımı',
          insertText:      'sınıf ${1:İsim} {\n\tfonksiyon başlangıç(${2}) {\n\t\t${3}\n\t}\n}',
        },
        {
          label:           'sınıf-kalıtım',
          documentation:   'Kalıtımla sınıf',
          insertText:      'sınıf ${1:Alt} kalıtım ${2:Üst} {\n\tfonksiyon başlangıç(${3}) {\n\t\t${4}\n\t}\n}',
        },
        {
          label:           'değişken',
          documentation:   'Değişken tanımla',
          insertText:      'değişken ${1:isim} = ${2:değer};',
        },
        {
          label:           'sabit',
          documentation:   'Sabit tanımla',
          insertText:      'sabit ${1:İSİM} = ${2:değer};',
        },
        {
          label:           'yaz',
          documentation:   'Ekrana yaz',
          insertText:      'yaz(${1:"merhaba"});',
        },
        {
          label:           'yazln',
          documentation:   'Satır sonu ile yaz',
          insertText:      'yazln(${1:"merhaba"});',
        },
        {
          label:           'hata-yakala',
          documentation:   'Hata yönetimi (DORUK\'ta try-catch yoktur; hata() kullanın)',
          insertText:      '// DORUK\'ta try-catch yoktur.\n// Girdi doğrulama için eğer bloğu kullanın:\neğer (${1:koşul}) {\n\thata("${2:hata mesajı}");\n}',
        },
      ].map(s => ({
        label:           s.label,
        kind:            SN,
        documentation:   s.documentation,
        insertText:      s.insertText,
        insertTextRules: RULE,
        range,
      }));

      return { suggestions: [...keywords, ...builtins, ...snippets] };
    },
  });

  // ─── Hover provider ────────────────────────────────────────────────────────
  const HOVER_DOCS = {
    'değişken':  '**değişken** — Değiştirilebilir değişken tanımlar.\n```drk\ndeğişken x = 42;\n```',
    'sabit':     '**sabit** — Değiştirilemez sabit tanımlar.\n```drk\nsabit PI = 3.14;\n```',
    'fonksiyon': '**fonksiyon** — Fonksiyon tanımlar.\n```drk\nfonksiyon topla(a, b) {\n  döndür a + b;\n}\n```',
    'sınıf':     '**sınıf** — Sınıf tanımlar.\n```drk\nsınıf Araba {\n  fonksiyon başlangıç(marka) {\n    bu.marka = marka;\n  }\n}\n```',
    'eğer':      '**eğer** — Koşullu dal.\n```drk\neğer (x > 0) {\n  yaz("pozitif");\n}\n```',
    'döngü':     '**döngü** — Koşul doğru iken döner.\n```drk\ndöngü (i < 10) {\n  i = i + 1;\n}\n```',
    'için':      '**için** — For döngüsü veya for-each.\n```drk\niçin (i = 0; i < 5; i = i + 1) { ... }\niçin (e içinde liste) { ... }\n```',
    'döndür':    '**döndür** — Fonksiyondan değer döndürür.',
    'yaz':       '**yaz(değer)** — Değeri standart çıktıya yazar (satır sonu yok).',
    'yazln':     '**yazln(değer)** — Değeri satır sonu ile yazar.',
    'oku':       '**oku()** — Kullanıcıdan bir satır girdi okur.',
    'hata':      '**hata(mesaj)** — Çalışma zamanı hatası fırlatır ve programı durdurur.',
    'uzunluk':   '**uzunluk(x)** — Liste, metin veya sözlüğün eleman sayısını döndürür.',
    'tip':       '**tip(x)** — Değerin tip adını metin olarak döndürür.',
    'tamSayı':   '**tamSayı(x)** — Değeri tam sayıya çevirir.',
    'ondalık':   '**ondalık(x)** — Değeri ondalıklı sayıya çevirir.',
    'metin':     '**metin(x)** — Değeri metne çevirir.',
    'ekle':      '**ekle(liste, eleman)** — Listeye eleman ekler.',
    'sil':       '**sil(liste, indis)** — Listeden indisteki elemanı siler.',
    'ters':      '**ters(liste)** — Listeyi ters çevirir.',
    'sırala':    '**sırala(liste)** — Listeyi sıralar.',
    'rastgele':  '**rastgele()** — 0 ile 1 arasında rastgele ondalıklı sayı döndürür.',
    'doğru':     '**doğru** — Mantıksal doğru değeri.',
    'yanlış':    '**yanlış** — Mantıksal yanlış değeri.',
    'boş':       '**boş** — Değersizlik (null) değeri.',
  };

  monaco.languages.registerHoverProvider('doruk', {
    provideHover(model, position) {
      const word = model.getWordAtPosition(position);
      if (!word) return null;
      const doc = HOVER_DOCS[word.word];
      if (!doc) return null;
      return {
        range: new monaco.Range(
          position.lineNumber, word.startColumn,
          position.lineNumber, word.endColumn,
        ),
        contents: [{ value: doc, isTrusted: true }],
      };
    },
  });

  // ─── Custom theme (VS Code Dark+ colour‑accurate) ──────────────────────────
  monaco.editor.defineTheme('doruk-dark', {
    base:    'vs-dark',
    inherit: true,
    rules: [
      // Comments
      { token: 'comment',               foreground: '6a9955', fontStyle: 'italic' },
      // Keywords (control flow)
      { token: 'keyword',               foreground: 'c586c0' },
      // Literals (true / false / null)
      { token: 'constant.language',     foreground: '569cd6' },
      // Built-in functions
      { token: 'support.function',      foreground: 'dcdcaa' },
      // Strings
      { token: 'string',                foreground: 'ce9178' },
      { token: 'string.quote',          foreground: 'ce9178' },
      { token: 'string.escape',         foreground: 'd7ba7d' },
      { token: 'string.escape.invalid', foreground: 'ff0000', fontStyle: 'bold' },
      // Numbers
      { token: 'number',                foreground: 'b5cea8' },
      { token: 'number.float',          foreground: 'b5cea8' },
      // Identifiers
      { token: 'identifier',            foreground: '9cdcfe' },
      // Operators
      { token: 'operator',              foreground: 'd4d4d4' },
      // Delimiters
      { token: 'delimiter.curly',       foreground: 'ffd700' },
      { token: 'delimiter.bracket',     foreground: 'ffd700' },
      { token: 'delimiter.parenthesis', foreground: 'ffd700' },
      { token: 'delimiter',             foreground: 'd4d4d4' },
    ],
    colors: {
      // Editor
      'editor.background':                    '#1e1e1e',
      'editor.foreground':                    '#d4d4d4',
      'editorLineNumber.foreground':          '#858585',
      'editorLineNumber.activeForeground':    '#c6c6c6',
      'editor.selectionBackground':           '#264f78',
      'editor.inactiveSelectionBackground':   '#3a3d41',
      'editor.selectionHighlightBackground':  '#add6ff26',
      'editor.wordHighlightBackground':       '#575757b8',
      'editor.wordHighlightStrongBackground': '#004972b8',
      'editorCursor.foreground':              '#aeafad',
      'editor.lineHighlightBackground':       '#2a2d2e',
      'editor.lineHighlightBorder':           '#282828',
      'editorIndentGuide.background':         '#404040',
      'editorIndentGuide.activeBackground':   '#707070',
      // Widget
      'editorWidget.background':              '#252526',
      'editorWidget.border':                  '#454545',
      'editorSuggestWidget.background':       '#252526',
      'editorSuggestWidget.border':           '#454545',
      'editorSuggestWidget.selectedBackground':'#062f4a',
      'editorHoverWidget.background':         '#252526',
      'editorHoverWidget.border':             '#454545',
      // Gutter
      'editorGutter.background':              '#1e1e1e',
      'editorGutter.addedBackground':         '#587c0c',
      'editorGutter.modifiedBackground':      '#0c7d9d',
      'editorGutter.deletedBackground':       '#94151b',
      // Scrollbar
      'scrollbar.shadow':                     '#000000',
      'scrollbarSlider.background':           '#42424266',
      'scrollbarSlider.hoverBackground':      '#646464b3',
      'scrollbarSlider.activeBackground':     '#bfbfbf66',
      // Minimap
      'minimap.background':                   '#1e1e1e',
      // Bracket match
      'editorBracketMatch.background':        '#0064001a',
      'editorBracketMatch.border':            '#888888',
      // Errors
      'editorError.foreground':               '#f44747',
      'editorWarning.foreground':             '#cca700',
      'editorInfo.foreground':                '#75beff',
    },
  });

  // ─── Document Formatter ────────────────────────────────────────────────────
  monaco.languages.registerDocumentFormattingEditProvider('doruk', {
    provideDocumentFormattingEdits(model) {
      const text = model.getValue();
      // Satır sonu boşlukları temizle, art arda 3+ boş satırı 2'ye indir
      const formatted = text.split('\n')
        .map(line => line.trimEnd())
        .join('\n')
        .replace(/\n{3,}/g, '\n\n');
      if (formatted === text) return [];
      return [{ range: model.getFullModelRange(), text: formatted }];
    },
  });

  console.log('[Doruk] Dil tanımı yüklendi.');
};
