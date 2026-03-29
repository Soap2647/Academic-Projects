'use strict';
/**
 * themes.js — Mert Studio Code renk temaları
 * 9 tema: CSS değişkenleri + Monaco editör renkleri
 */

window.MSC_THEMES = [
  {
    id: 'gece-kadifesi',
    name: 'Gece Kadifesi',
    desc: 'Yumuşak koyu tonlar',
    base: 'vs-dark',
    preview: ['#569cd6','#ce9178','#4ec9b0','#dcdcaa','#c586c0'],
    css: {
      '--bg':'#1e1e1e','--bg-alt':'#252526','--bg-hover':'#2a2d2e','--bg-active':'#37373d',
      '--fg':'#cccccc','--fg-dim':'#858585','--border':'#474747','--border-light':'#3c3c3c',
      '--actbar-bg':'#333333','--sidebar-bg':'#252526',
      '--titlebar-bg':'#3c3c3c','--titlebar-fg':'#ccc',
      '--tab-inactive-bg':'#2d2d2d','--tab-active-bg':'#1e1e1e','--tab-active-border':'#007acc',
      '--panel-bg':'#1e1e1e','--statusbar-bg':'#007acc','--statusbar-fg':'#ffffff',
      '--btn-bg':'#0e639c','--btn-hover':'#1177bb',
    },
    monaco: {
      base:'vs-dark', bg:'#1e1e1e', fg:'#d4d4d4',
      kw:'#569cd6', str:'#ce9178', num:'#b5cea8', cmt:'#6a9955',
      fn:'#dcdcaa', lit:'#4ec9b0', sel:'#264f78', line:'#2a2d2e',
    },
  },
  {
    id: 'gun-isigi',
    name: 'Gün Işığı',
    desc: 'Temiz ve aydınlık',
    base: 'vs',
    preview: ['#0000ff','#a31515','#098658','#795e26','#af00db'],
    css: {
      '--bg':'#ffffff','--bg-alt':'#f3f3f3','--bg-hover':'#e8e8e8','--bg-active':'#d4d4d4',
      '--fg':'#1f1f1f','--fg-dim':'#6e6e6e','--border':'#c8c8c8','--border-light':'#e0e0e0',
      '--actbar-bg':'#2c2c2c','--sidebar-bg':'#f3f3f3',
      '--titlebar-bg':'#dddddd','--titlebar-fg':'#333',
      '--tab-inactive-bg':'#ececec','--tab-active-bg':'#ffffff','--tab-active-border':'#0066bf',
      '--panel-bg':'#f3f3f3','--statusbar-bg':'#0066bf','--statusbar-fg':'#ffffff',
      '--btn-bg':'#0066bf','--btn-hover':'#0057a8',
    },
    monaco: {
      base:'vs', bg:'#ffffff', fg:'#1f1f1f',
      kw:'#0000ff', str:'#a31515', num:'#098658', cmt:'#008000',
      fn:'#795e26', lit:'#0070c1', sel:'#add6ff', line:'#f0f0f0',
    },
  },
  {
    id: 'ates-dansi',
    name: 'Ateş Dansı',
    desc: 'Sıcak ve canlı renkler',
    base: 'vs-dark',
    preview: ['#ff6b6b','#ffd93d','#6bcb77','#4d96ff','#c77dff'],
    css: {
      '--bg':'#1a0a00','--bg-alt':'#260f00','--bg-hover':'#2e1500','--bg-active':'#3d1d00',
      '--fg':'#ffd8b1','--fg-dim':'#a87050','--border':'#5c2e00','--border-light':'#3d1d00',
      '--actbar-bg':'#1f0c00','--sidebar-bg':'#200d00',
      '--titlebar-bg':'#2e1500','--titlebar-fg':'#ffd8b1',
      '--tab-inactive-bg':'#261000','--tab-active-bg':'#1a0a00','--tab-active-border':'#ff6b00',
      '--panel-bg':'#1a0a00','--statusbar-bg':'#c94f00','--statusbar-fg':'#ffffff',
      '--btn-bg':'#c94f00','--btn-hover':'#e05800',
    },
    monaco: {
      base:'vs-dark', bg:'#1a0a00', fg:'#ffd8b1',
      kw:'#ff6b6b', str:'#ffd93d', num:'#6bcb77', cmt:'#7a6050',
      fn:'#ffb347', lit:'#4d96ff', sel:'#5c2e00', line:'#2e1500',
    },
  },
  {
    id: 'karanlik-mor',
    name: 'Karanlık Mor',
    desc: 'Mor ve pembe tonlar',
    base: 'vs-dark',
    preview: ['#c792ea','#f78c6c','#80cbc4','#ffcb6b','#89ddff'],
    css: {
      '--bg':'#0f0e17','--bg-alt':'#1a1825','--bg-hover':'#211f2e','--bg-active':'#2a2740',
      '--fg':'#e8e3f0','--fg-dim':'#8b87a0','--border':'#3d3a52','--border-light':'#2a2740',
      '--actbar-bg':'#150e2a','--sidebar-bg':'#130d24',
      '--titlebar-bg':'#1a1530','--titlebar-fg':'#c9b8e8',
      '--tab-inactive-bg':'#180f2e','--tab-active-bg':'#0f0e17','--tab-active-border':'#c792ea',
      '--panel-bg':'#0f0e17','--statusbar-bg':'#7c3aed','--statusbar-fg':'#ffffff',
      '--btn-bg':'#7c3aed','--btn-hover':'#8b5cf6',
    },
    monaco: {
      base:'vs-dark', bg:'#0f0e17', fg:'#e8e3f0',
      kw:'#c792ea', str:'#f78c6c', num:'#80cbc4', cmt:'#5c5470',
      fn:'#ffcb6b', lit:'#89ddff', sel:'#2a2740', line:'#1a1825',
    },
  },
  {
    id: 'kutup-mavisi',
    name: 'Kutup Mavisi',
    desc: 'Soğuk kutup tonları',
    base: 'vs-dark',
    preview: ['#88c0d0','#81a1c1','#a3be8c','#ebcb8b','#b48ead'],
    css: {
      '--bg':'#2e3440','--bg-alt':'#3b4252','--bg-hover':'#434c5e','--bg-active':'#4c566a',
      '--fg':'#eceff4','--fg-dim':'#7b88a1','--border':'#4c566a','--border-light':'#3b4252',
      '--actbar-bg':'#2e3440','--sidebar-bg':'#2e3440',
      '--titlebar-bg':'#282c34','--titlebar-fg':'#d8dee9',
      '--tab-inactive-bg':'#3b4252','--tab-active-bg':'#2e3440','--tab-active-border':'#88c0d0',
      '--panel-bg':'#2e3440','--statusbar-bg':'#5e81ac','--statusbar-fg':'#eceff4',
      '--btn-bg':'#5e81ac','--btn-hover':'#81a1c1',
    },
    monaco: {
      base:'vs-dark', bg:'#2e3440', fg:'#eceff4',
      kw:'#81a1c1', str:'#a3be8c', num:'#b48ead', cmt:'#616e88',
      fn:'#88c0d0', lit:'#8fbcbb', sel:'#434c5e', line:'#3b4252',
    },
  },
  {
    id: 'gunes-tutulmasi',
    name: 'Güneş Tutulması',
    desc: 'Bilimsel renk paleti',
    base: 'vs-dark',
    preview: ['#00d2ff','#ff6b9d','#c8ff00','#ff9a00','#7c5cbf'],
    css: {
      '--bg':'#060b14','--bg-alt':'#0a1220','--bg-hover':'#0e192c','--bg-active':'#14243d',
      '--fg':'#c5d8ff','--fg-dim':'#4a6080','--border':'#1a2e48','--border-light':'#0e192c',
      '--actbar-bg':'#040810','--sidebar-bg':'#050a12',
      '--titlebar-bg':'#070d1a','--titlebar-fg':'#7aadff',
      '--tab-inactive-bg':'#08101e','--tab-active-bg':'#060b14','--tab-active-border':'#00d2ff',
      '--panel-bg':'#060b14','--statusbar-bg':'#005f99','--statusbar-fg':'#c5d8ff',
      '--btn-bg':'#005f99','--btn-hover':'#0075bf',
    },
    monaco: {
      base:'vs-dark', bg:'#060b14', fg:'#c5d8ff',
      kw:'#00d2ff', str:'#ff6b9d', num:'#c8ff00', cmt:'#2a4060',
      fn:'#ff9a00', lit:'#7c5cbf', sel:'#14243d', line:'#0a1220',
    },
  },
  {
    id: 'okyanus',
    name: 'Okyanus',
    desc: 'Derin mavi dalgalar',
    base: 'vs-dark',
    preview: ['#4dd0e1','#80cbc4','#a5d6a7','#fff176','#ef9a9a'],
    css: {
      '--bg':'#002b36','--bg-alt':'#073642','--bg-hover':'#0d4050','--bg-active':'#165060',
      '--fg':'#93a1a1','--fg-dim':'#586e75','--border':'#165060','--border-light':'#073642',
      '--actbar-bg':'#002028','--sidebar-bg':'#002030',
      '--titlebar-bg':'#001820','--titlebar-fg':'#93a1a1',
      '--tab-inactive-bg':'#002535','--tab-active-bg':'#002b36','--tab-active-border':'#2aa198',
      '--panel-bg':'#002b36','--statusbar-bg':'#268bd2','--statusbar-fg':'#fdf6e3',
      '--btn-bg':'#268bd2','--btn-hover':'#2196f3',
    },
    monaco: {
      base:'vs-dark', bg:'#002b36', fg:'#93a1a1',
      kw:'#268bd2', str:'#2aa198', num:'#859900', cmt:'#586e75',
      fn:'#b58900', lit:'#cb4b16', sel:'#165060', line:'#073642',
    },
  },
  {
    id: 'gun-batimi',
    name: 'Gün Batımı',
    desc: 'Sıcak turuncu-pembe',
    base: 'vs-dark',
    preview: ['#ffb347','#ff6b9d','#a8edea','#fed6e3','#f093fb'],
    css: {
      '--bg':'#1a0e1c','--bg-alt':'#251328','--bg-hover':'#2f1933','--bg-active':'#3d2244',
      '--fg':'#f8d7e3','--fg-dim':'#8a5b72','--border':'#4a2855','--border-light':'#2f1933',
      '--actbar-bg':'#150b18','--sidebar-bg':'#180c1b',
      '--titlebar-bg':'#200f24','--titlebar-fg':'#f8d7e3',
      '--tab-inactive-bg':'#1e0f22','--tab-active-bg':'#1a0e1c','--tab-active-border':'#ff6b9d',
      '--panel-bg':'#1a0e1c','--statusbar-bg':'#c2185b','--statusbar-fg':'#ffffff',
      '--btn-bg':'#c2185b','--btn-hover':'#d81b60',
    },
    monaco: {
      base:'vs-dark', bg:'#1a0e1c', fg:'#f8d7e3',
      kw:'#f093fb', str:'#ffb347', num:'#a8edea', cmt:'#6a3a55',
      fn:'#fed6e3', lit:'#ff6b9d', sel:'#3d2244', line:'#251328',
    },
  },
  {
    id: 'orman',
    name: 'Orman',
    desc: 'Doğal yeşil huzur',
    base: 'vs-dark',
    preview: ['#8bc34a','#4caf50','#cddc39','#ffeb3b','#00bcd4'],
    css: {
      '--bg':'#0a110a','--bg-alt':'#0f1a0f','--bg-hover':'#152115','--bg-active':'#1c2d1c',
      '--fg':'#c8e6c9','--fg-dim':'#5a7a5a','--border':'#2d4a2d','--border-light':'#1c2d1c',
      '--actbar-bg':'#081008','--sidebar-bg':'#090f09',
      '--titlebar-bg':'#0c160c','--titlebar-fg':'#a5d6a7',
      '--tab-inactive-bg':'#0d160d','--tab-active-bg':'#0a110a','--tab-active-border':'#4caf50',
      '--panel-bg':'#0a110a','--statusbar-bg':'#2e7d32','--statusbar-fg':'#c8e6c9',
      '--btn-bg':'#2e7d32','--btn-hover':'#388e3c',
    },
    monaco: {
      base:'vs-dark', bg:'#0a110a', fg:'#c8e6c9',
      kw:'#8bc34a', str:'#cddc39', num:'#ffeb3b', cmt:'#3a5a3a',
      fn:'#4caf50', lit:'#00bcd4', sel:'#1c2d1c', line:'#0f1a0f',
    },
  },
];

// Apply a theme by ID
window.mscApplyTheme = function(themeId) {
  const theme = window.MSC_THEMES.find(t => t.id === themeId);
  if (!theme) return;

  // Apply CSS variables
  const root = document.documentElement;
  Object.entries(theme.css).forEach(([k, v]) => root.style.setProperty(k, v));

  // Apply Monaco theme if editor is ready
  if (window._monacoReady && typeof monaco !== 'undefined') {
    const m = theme.monaco;
    const rules = [
      { token: 'keyword',           foreground: m.kw.replace('#',''),  fontStyle: 'bold' },
      { token: 'constant.language', foreground: m.lit.replace('#','') },
      { token: 'support.function',  foreground: m.fn.replace('#','')  },
      { token: 'string',            foreground: m.str.replace('#','') },
      { token: 'string.quote',      foreground: m.str.replace('#','') },
      { token: 'string.escape',     foreground: m.num.replace('#','') },
      { token: 'number',            foreground: m.num.replace('#','') },
      { token: 'number.float',      foreground: m.num.replace('#','') },
      { token: 'comment',           foreground: m.cmt.replace('#',''), fontStyle: 'italic' },
      { token: 'identifier',        foreground: m.fg.replace('#','')  },
      { token: 'delimiter.curly',   foreground: 'ffd700' },
      { token: 'delimiter.bracket', foreground: 'ffd700' },
      { token: 'delimiter.parenthesis', foreground: 'ffd700' },
    ];
    monaco.editor.defineTheme('msc-' + themeId, {
      base:    theme.base,
      inherit: true,
      rules,
      colors: {
        'editor.background':               m.bg,
        'editor.foreground':               m.fg,
        'editorLineNumber.foreground':     m.cmt,
        'editor.selectionBackground':      m.sel,
        'editor.lineHighlightBackground':  m.line,
        'editorWidget.background':         theme.css['--bg-alt'],
        'editorSuggestWidget.background':  theme.css['--bg-alt'],
        'scrollbarSlider.background':      theme.css['--bg-active'] + '88',
        'minimap.background':              m.bg,
      },
    });
    monaco.editor.setTheme('msc-' + themeId);
  }

  // Persist
  localStorage.setItem('msc-theme', themeId);
};
