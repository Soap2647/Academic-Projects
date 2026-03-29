'use strict';
/**
 * app.js — Mert Studio Code
 * Tam fonksiyonel IDE: editör, dosya gezgini, arama, terminal, tema seçici, ayarlar
 */

// ─── State ────────────────────────────────────────────────────────────────────
const state = {
  editor:           null,
  tabs:             [],
  activeTabIdx:     -1,
  openFolderPath:   null,
  treeExpanded:     new Set(),
  treeData:         [],
  isRunning:        false,
  activeSidePanel:  'explorer',
  activeBottomTab:  'output',
  panelVisible:     true,
  recentFiles:      [],
  problems:         [],
  errorCount:       0,
  warnCount:        0,
  currentThemeId:   'gece-kadifesi',
  outputBuffer:     '',
  terminalStarted:  false,
  searchCaseSensitive: false,
  settings: {
    fontSize:     14,
    fontFamily:   '"Cascadia Code",Consolas,monospace',
    tabSize:      4,
    wordWrap:     false,
    lineNumbers:  true,
    minimap:      true,
    bracketColor: true,
    autosave:     3000,
    formatOnSave: true,
  },
  autosaveTimer: null,
};

const $  = id  => document.getElementById(id);
const $$ = sel => document.querySelectorAll(sel);

// ─── Utility ──────────────────────────────────────────────────────────────────
function basename(p) { return p.replace(/\\/g,'/').split('/').pop() || p; }
function dirname(p)  { const s = p.replace(/\\/g,'/'); return s.substring(0, s.lastIndexOf('/')); }
function extname(p)  { const b = basename(p); const i = b.lastIndexOf('.'); return i < 0 ? '' : b.slice(i).toLowerCase(); }

function notify(msg, type='info', ms=3500) {
  const el = document.createElement('div');
  el.className = `notif notif-${type}`;
  el.innerHTML = `<span class="notif-msg">${msg}</span>`;
  $('notification-area').appendChild(el);
  setTimeout(() => { el.style.opacity='0'; el.style.transition='opacity 0.3s'; setTimeout(() => el.remove(), 300); }, ms);
}

function fileIconHtml(name, isDir) {
  if (isDir) return `<span class="tree-icon icon-dir"><svg viewBox="0 0 24 24"><path d="M20 6h-8l-2-2H4c-1.1 0-2 .9-2 2v12c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2z"/></svg></span>`;
  const ext = extname(name);
  const cls = {'.drk':'icon-drk','.js':'icon-js','.ts':'icon-ts','.json':'icon-json','.cpp':'icon-cpp','.h':'icon-h','.md':'icon-md','.txt':'icon-txt'}[ext]||'icon-other';
  const svg = ext==='.drk'
    ? '<svg viewBox="0 0 24 24"><path d="M14 2H6c-1.1 0-2 .9-2 2v16c0 1.1.9 2 2 2h12c1.1 0 2-.9 2-2V8l-6-6zm4 18H6V4h7v5h5v11z"/></svg>'
    : '<svg viewBox="0 0 24 24"><path d="M6 2c-1.1 0-2 .9-2 2v16c0 1.1.9 2 2 2h12c1.1 0 2-.9 2-2V8l-6-6H6zm7 1.5L18.5 9H13V3.5z"/></svg>';
  return `<span class="tree-icon ${cls}">${svg}</span>`;
}

function guessLang(p) {
  return {'.js':'javascript','.ts':'typescript','.json':'json','.cpp':'cpp','.h':'cpp','.c':'c','.md':'markdown','.html':'html','.css':'css','.py':'python'}[extname(p)] || 'plaintext';
}

// ─── Recent ────────────────────────────────────────────────────────────────────
function loadRecent() { try { state.recentFiles = JSON.parse(localStorage.getItem('msc-recent')||'[]'); } catch { state.recentFiles=[]; } }
function saveRecent() { try { localStorage.setItem('msc-recent', JSON.stringify(state.recentFiles.slice(0,15))); } catch {} }
function pushRecent(path) {
  state.recentFiles = state.recentFiles.filter(r=>r.path!==path);
  state.recentFiles.unshift({ path, name: basename(path) });
  saveRecent(); renderWelcomeRecent();
}
function renderWelcomeRecent() {
  const el = $('recent-list'); if (!el) return;
  if (!state.recentFiles.length) { el.innerHTML='<span class="recent-empty">Henüz açılan dosya yok.</span>'; return; }
  el.innerHTML = state.recentFiles.map(r =>
    `<div class="recent-item" data-path="${r.path}"><span class="recent-name">${r.name}</span><span class="recent-path">${dirname(r.path)}</span></div>`
  ).join('');
  el.querySelectorAll('.recent-item').forEach(e => e.addEventListener('click', () => openFile(e.dataset.path)));
}

// ─── Monaco ortam yapılandırması (Electron için gerekli) ──────────────────────
window.MonacoEnvironment = {
  getWorkerUrl: function(_moduleId, _label) {
    return '../node_modules/monaco-editor/min/vs/base/worker/workerMain.js';
  }
};

// ─── Monaco init ───────────────────────────────────────────────────────────────
function initMonaco() {
  return new Promise((resolve, reject) => {
    // 15 saniye içinde yüklenmezse hata ver
    const timeout = setTimeout(() => {
      reject(new Error(
        'Monaco Editor yüklenemedi (15s zaman aşımı).\n' +
        'node_modules/monaco-editor klasörünün var olduğundan emin olun.\n' +
        'npm install komutunu çalıştırmayı deneyin.'
      ));
    }, 15000);

    if (typeof require === 'undefined' || typeof require.config === 'undefined') {
      clearTimeout(timeout);
      reject(new Error(
        'AMD yükleyici (loader.js) bulunamadı.\n' +
        'node_modules/monaco-editor/min/vs/loader.js dosyası eksik olabilir.'
      ));
      return;
    }

    require.config({ paths: { vs: '../node_modules/monaco-editor/min/vs' } });
    require(['vs/editor/editor.main'], () => {
      clearTimeout(timeout);
      try {
        window._monacoReady = true;
        if (window.registerDorukLanguage) window.registerDorukLanguage(monaco);

        // Apply saved theme
        const savedTheme = localStorage.getItem('msc-theme') || 'gece-kadifesi';
        window.mscApplyTheme(savedTheme);
        state.currentThemeId = savedTheme;

        state.editor = monaco.editor.create($('editor-mount'), {
          model: null,
          theme: 'msc-' + savedTheme,
          fontSize:     state.settings.fontSize,
          fontFamily:   state.settings.fontFamily,
          fontLigatures: true,
          lineNumbers:  state.settings.lineNumbers ? 'on' : 'off',
          minimap:      { enabled: state.settings.minimap },
          scrollBeyondLastLine: false,
          wordWrap:     state.settings.wordWrap ? 'on' : 'off',
          tabSize:      state.settings.tabSize,
          insertSpaces: true,
          renderWhitespace: 'selection',
          cursorBlinking: 'smooth', cursorSmoothCaretAnimation: 'on',
          smoothScrolling: true, renderLineHighlight: 'all',
          bracketPairColorization: { enabled: state.settings.bracketColor },
          guides: { bracketPairs: 'active', indentation: true },
          suggestOnTriggerCharacters: true, quickSuggestions: true,
          snippetSuggestions: 'inline', formatOnPaste: true,
          padding: { top: 8, bottom: 8 },
          scrollbar: { verticalScrollbarSize: 10, horizontalScrollbarSize: 10 },
          occurrencesHighlight: true, selectionHighlight: true,
        });

        state.editor.onDidChangeCursorPosition(e => updateStatusPos(e.position));
        state.editor.onDidChangeCursorSelection(e => updateStatusSelection(state.editor.getSelection(), state.editor.getModel()));
        state.editor.onDidChangeModelContent(() => {
          const tab = state.tabs[state.activeTabIdx];
          if (tab && !tab.modified) { tab.modified = true; renderTabs(); }
          scheduleAutosave();
        });

        state.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS,       () => saveActiveFile());
        state.editor.addCommand(monaco.KeyCode.F5,                                  () => runDoruk());
        state.editor.addCommand(monaco.KeyMod.Shift | monaco.KeyCode.F5,            () => stopDoruk());
        state.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Backquote,   () => switchBottomTab('terminal'));

        window.addEventListener('resize', () => state.editor.layout());
        resolve();
      } catch(err) {
        reject(err);
      }
    });
  });
}

// ─── Autosave ─────────────────────────────────────────────────────────────────
function scheduleAutosave() {
  if (!state.settings.autosave || state.settings.autosave === 'off') return;
  clearTimeout(state.autosaveTimer);
  state.autosaveTimer = setTimeout(() => {
    const tab = state.tabs[state.activeTabIdx];
    if (tab && tab.modified && !tab.path.startsWith('untitled-')) saveActiveFile(true);
  }, +state.settings.autosave);
}

// ─── Tabs ─────────────────────────────────────────────────────────────────────
function renderTabs() {
  const list = $('tabs-list');
  list.innerHTML = '';
  if (!state.tabs.length) { showWelcomeScreen(); return; }
  state.tabs.forEach((tab, i) => {
    const div = document.createElement('div');
    div.className = 'tab'+(i===state.activeTabIdx?' active':'')+(tab.modified?' modified':'');
    div.title = tab.path;
    div.innerHTML = `<span class="tab-name">${tab.name}</span><span class="tab-close" data-idx="${i}"></span>`;
    div.addEventListener('click', e => { if (!e.target.classList.contains('tab-close')) switchTab(i); });
    div.querySelector('.tab-close').addEventListener('click', e => { e.stopPropagation(); closeTab(i); });
    div.addEventListener('mousedown', e => { if (e.button===1) { e.preventDefault(); closeTab(i); }});
    div.addEventListener('contextmenu', e => { e.preventDefault(); showTabContextMenu(e, i); });
    list.appendChild(div);
  });
  list.querySelector('.tab.active')?.scrollIntoView({ block:'nearest', inline:'center' });
}

function switchTab(idx) {
  const prev = state.tabs[state.activeTabIdx];
  if (prev && state.editor.getModel()) prev.viewState = state.editor.saveViewState();
  state.activeTabIdx = idx;
  const tab = state.tabs[idx];
  showEditorMount();
  state.editor.setModel(tab.model);
  if (tab.viewState) state.editor.restoreViewState(tab.viewState);
  state.editor.focus();
  updateBreadcrumb(tab.path); updateStatusLang(tab.path); updateStatusPos(state.editor.getPosition());
  updateTitleBarFile(tab.name); updateStatusIndent();
  renderTabs(); highlightActiveInTree(tab.path);
  renderOutline();
}

function closeTab(idx) {
  const tab = state.tabs[idx];
  if (tab.modified && !confirm(`"${tab.name}" kaydedilmemiş. Kapatılsın mı?`)) return;
  tab.model.dispose(); state.tabs.splice(idx, 1);
  if (!state.tabs.length) { state.activeTabIdx=-1; showWelcomeScreen(); }
  else switchTab(Math.min(idx, state.tabs.length-1));
  renderTabs();
}

function showWelcomeScreen() {
  $('welcome-screen').style.display = 'flex';
  $('editor-mount').style.display   = 'none';
  $('breadcrumb-content').textContent = '';
  updateStatusPos(null);
  updateTitleBarFile(null);
  renderOutline();
}
function showEditorMount() {
  $('welcome-screen').style.display = 'none';
  $('editor-mount').style.display   = 'block';
  state.editor?.layout();
}

// ─── File opening ──────────────────────────────────────────────────────────────
async function openFile(filePath) {
  if (!window._monacoReady) { notify('Editör henüz yükleniyor, lütfen bekleyin...', 'info', 2000); return; }
  const existing = state.tabs.findIndex(t => t.path === filePath);
  if (existing >= 0) { switchTab(existing); return; }
  const res = await window.dorukAPI.fileRead(filePath);
  if (!res.ok) { notify(`Dosya açılamadı: ${res.error}`, 'error'); return; }
  const lang  = extname(filePath) === '.drk' ? 'doruk' : guessLang(filePath);
  const model = monaco.editor.createModel(res.content, lang, monaco.Uri.file(filePath));
  state.tabs.push({ path: filePath, name: basename(filePath), model, modified: false, viewState: null });
  pushRecent(filePath);
  // Klasör açık değilse üst dizini otomatik aç
  if (!state.openFolderPath) {
    await openFolder(dirname(filePath), true);
    if (state.activeSidePanel !== 'explorer') switchSidePanel('explorer');
  }
  switchTab(state.tabs.length - 1);
  // Dosyayı ağaçta göster
  setTimeout(() => highlightActiveInTree(filePath), 80);
}

async function openFileDialog()   { const paths = await window.dorukAPI.dialogOpenFile(); for (const p of paths) await openFile(p); }
async function openFolderDialog() { const p = await window.dorukAPI.dialogOpenFolder(); if (p) await openFolder(p); }

async function openFolder(folderPath, silent=false) {
  state.openFolderPath = folderPath;
  $('folder-label-bar').classList.remove('hidden');
  $('folder-label-text').textContent = basename(folderPath);
  $('no-folder-hint').style.display  = 'none';
  state.treeData = await window.dorukAPI.dirReadDeep(folderPath);
  renderFileTree();
  if (!silent) notify(`📂 ${basename(folderPath)} açıldı`, 'success', 2000);
}

// ─── File tree ────────────────────────────────────────────────────────────────
function renderFileTree() {
  const container = $('file-tree');
  const hint      = $('no-folder-hint');
  // Eski ağaç UL'larını temizle, hint'e dokunma
  Array.from(container.children).forEach(child => { if (child !== hint) child.remove(); });
  if (state.treeData && state.treeData.length) {
    if (hint) hint.style.display = 'none';
    container.insertAdjacentElement('afterbegin', buildTreeNodes(state.treeData, 0));
  } else {
    if (hint) hint.style.display = '';
  }
}

function buildTreeNodes(entries, depth) {
  const ul = document.createElement('ul');
  ul.style.cssText = 'list-style:none;padding:0;';
  for (const entry of entries) {
    const li  = document.createElement('li');
    const row = document.createElement('div');
    row.className    = 'tree-item';
    row.dataset.path = entry.path;
    row.style.paddingLeft = (depth * 12 + 8) + 'px';
    if (entry.isDir) {
      const open = state.treeExpanded.has(entry.path);
      row.innerHTML = `<span class="tree-arrow${open?' open':''}">▶</span>${fileIconHtml(entry.name,true)}<span class="tree-label">${entry.name}</span>`;
      const wrap = document.createElement('div');
      wrap.className = 'tree-children' + (open ? ' open' : '');
      if (entry.children && open) wrap.appendChild(buildTreeNodes(entry.children, depth+1));
      row.addEventListener('click', async () => {
        if (state.treeExpanded.has(entry.path)) {
          state.treeExpanded.delete(entry.path);
          row.querySelector('.tree-arrow').classList.remove('open');
          wrap.classList.remove('open'); wrap.innerHTML='';
        } else {
          state.treeExpanded.add(entry.path);
          row.querySelector('.tree-arrow').classList.add('open');
          const ch = await window.dorukAPI.dirReadDeep(entry.path);
          entry.children = ch; wrap.innerHTML='';
          wrap.appendChild(buildTreeNodes(ch, depth+1)); wrap.classList.add('open');
        }
      });
      li.append(row, wrap);
    } else {
      row.innerHTML = `<span class="tree-arrow" style="visibility:hidden">▶</span>${fileIconHtml(entry.name,false)}<span class="tree-label">${entry.name}</span>`;
      row.addEventListener('click', () => openFile(entry.path));
      li.appendChild(row);
    }
    row.addEventListener('contextmenu', e => { e.preventDefault(); showTreeContextMenu(e, entry); });
    ul.appendChild(li);
  }
  return ul;
}

function highlightActiveInTree(fp) {
  $$('.tree-item.active').forEach(e => e.classList.remove('active'));
  const el = document.querySelector(`.tree-item[data-path="${CSS.escape(fp)}"]`);
  if (el) { el.classList.add('active'); el.scrollIntoView({ block: 'nearest' }); }
}
async function refreshTree() { if (state.openFolderPath) { state.treeData = await window.dorukAPI.dirReadDeep(state.openFolderPath); renderFileTree(); } }

// ─── Saving ────────────────────────────────────────────────────────────────────
async function saveActiveFile(silent=false) {
  const tab = state.tabs[state.activeTabIdx]; if (!tab) return;
  // Kaydetmeden önce formatla
  if (state.settings.formatOnSave && state.editor && !tab.path.startsWith('untitled-')) {
    state.editor.trigger('app', 'editor.action.formatDocument', null);
    await new Promise(r => setTimeout(r, 40));
  }
  const content = tab.model.getValue();
  if (tab.path.startsWith('untitled-')) {
    const res = await window.dorukAPI.dialogSaveAs(content);
    if (!res.ok) return;
    tab.path = res.path; tab.name = basename(res.path);
    const old = tab.model;
    tab.model = monaco.editor.createModel(content, guessLang(res.path), monaco.Uri.file(res.path));
    state.editor.setModel(tab.model); old.dispose(); pushRecent(res.path);
  } else {
    const res = await window.dorukAPI.fileWrite(tab.path, content);
    if (!res.ok) { notify(`Kaydedilemedi: ${res.error}`, 'error'); return; }
  }
  tab.modified = false; renderTabs();
  if (!silent) notify(`✔ ${tab.name} kaydedildi`, 'success', 1800);
  // Sessiz canlı analiz (.drk dosyaları için)
  if (extname(tab.path) === '.drk') liveAnalyze(tab.path);
}

async function saveAllFiles() {
  for (let i=0; i<state.tabs.length; i++) {
    if (state.tabs[i].modified) { const p=state.activeTabIdx; state.activeTabIdx=i; await saveActiveFile(true); state.activeTabIdx=p; }
  }
  notify('Tüm dosyalar kaydedildi.', 'success', 1800);
}

async function newUntitledFile() {
  if (!window._monacoReady) { notify('Editör henüz yükleniyor, lütfen bekleyin...', 'info', 2000); return; }
  const idx = state.tabs.filter(t=>t.path.startsWith('untitled-')).length + 1;
  const path = `untitled-${idx}.drk`;
  const model = monaco.editor.createModel('', 'doruk', monaco.Uri.parse(`file:///untitled-${idx}.drk`));
  state.tabs.push({ path, name: `untitled-${idx}.drk`, model, modified: false, viewState: null });
  switchTab(state.tabs.length - 1);
}

// ─── Run / Stop ───────────────────────────────────────────────────────────────
async function runDoruk() {
  const tab = state.tabs[state.activeTabIdx];
  if (!tab)                          { notify('Önce bir DORUK dosyası açın.', 'warning'); return; }
  if (tab.path.startsWith('untitled-')) { notify('Önce dosyayı kaydedin (Ctrl+S).', 'warning'); return; }
  if (extname(tab.path) !== '.drk')  { notify('Sadece .drk dosyaları çalıştırılabilir.', 'warning'); return; }
  if (tab.modified) await saveActiveFile(true);
  clearOutput(); setRunState(true); switchBottomTab('output');
  window.dorukAPI.dorukRun(tab.path);
}
function stopDoruk() { window.dorukAPI.dorukStop(); setRunState(false); }
function setRunState(running) {
  state.isRunning = running;
  $('btn-run').disabled  = running; $('btn-stop').disabled = !running;
  if ($('btn-run-big'))  $('btn-run-big').disabled  = running;
  if ($('btn-stop-big')) $('btn-stop-big').disabled = !running;
  $('status-running').style.display = running ? 'flex' : 'none';
  $('status-bar').style.background  = running ? 'var(--statusbar-running)' : 'var(--statusbar-bg)';
}
function initRunnerEvents() {
  window.dorukAPI.onDorukOutput(d => appendOutput(d.text, d.type));
  window.dorukAPI.onDorukExit(() => { setRunState(false); parseProblems(); });
}

// ─── Output ───────────────────────────────────────────────────────────────────
function clearOutput() { state.outputBuffer=''; $('output-container').innerHTML=''; }
function appendOutput(text, type) {
  state.outputBuffer += text;
  const span = document.createElement('span');
  span.className = { info:'out-info', stdout:'out-stdout', stderr:'out-stderr', error:'out-error', success:'out-success' }[type] || 'out-stdout';
  span.textContent = text;
  const c = $('output-container'); c.appendChild(span); c.scrollTop = c.scrollHeight;
}

// ─── Problems ──────────────────────────────────────────────────────────────────
function parseProblems() {
  const re = /\[(HATA|UYARI)\] ([^:]+):(\d+):(\d+) → (.+)/g;
  state.problems = []; let m;
  while ((m = re.exec(state.outputBuffer)) !== null)
    state.problems.push({ type: m[1]==='HATA'?'error':'warning', file:m[2], line:+m[3], col:+m[4], msg:m[5] });
  updateProblemsPanel(); updateEditorMarkers();
  state.errorCount = state.problems.filter(p=>p.type==='error').length;
  state.warnCount  = state.problems.filter(p=>p.type==='warning').length;
  updateStatusDiag();
}
function updateProblemsPanel() {
  const c = $('problems-container');
  if (!state.problems.length) { c.innerHTML='<div class="no-problems">✔ Herhangi bir sorun bulunamadı.</div>'; return; }
  c.innerHTML = state.problems.map(p => `
    <div class="problem-item">
      <span class="problem-${p.type}">${p.type==='error'?'⊗':'⚠'}</span>
      <span class="problem-msg">${p.msg}</span>
      <span class="problem-file">${p.file}:${p.line}:${p.col}</span>
    </div>`).join('');
  c.querySelectorAll('.problem-item').forEach((el,i) => {
    el.addEventListener('click', () => {
      const prob = state.problems[i];
      const tab  = state.tabs.find(t => t.path.endsWith(prob.file) || basename(t.path)===prob.file);
      if (tab) { switchTab(state.tabs.indexOf(tab)); state.editor.revealLineInCenter(prob.line); state.editor.setPosition({ lineNumber:prob.line, column:prob.col }); state.editor.focus(); }
    });
  });
}
function updateEditorMarkers() {
  state.tabs.forEach(tab => {
    const markers = state.problems
      .filter(p => tab.path.endsWith(p.file) || basename(tab.path)===p.file)
      .map(p => ({ severity: p.type==='error' ? monaco.MarkerSeverity.Error : monaco.MarkerSeverity.Warning, startLineNumber:p.line, endLineNumber:p.line, startColumn:p.col, endColumn:999, message:p.msg, source:'doruk' }));
    monaco.editor.setModelMarkers(tab.model, 'doruk', markers);
  });
}

// ─── FILE SEARCH ──────────────────────────────────────────────────────────────
let searchDebounce = null;
async function runSearch(query) {
  const resWrap = $('search-results-wrap');
  const status  = $('search-status');
  if (!query) { resWrap.innerHTML=''; status.textContent=''; return; }
  if (!state.openFolderPath) { status.textContent='Klasör açılmamış.'; return; }
  status.textContent = 'Aranıyor...';
  resWrap.innerHTML  = '';

  const results = await window.dorukAPI.searchFiles(state.openFolderPath, query, state.searchCaseSensitive);

  if (!results.length) { status.textContent = `"${query}" için sonuç bulunamadı.`; return; }

  // Group by file
  const byFile = {};
  results.forEach(r => { (byFile[r.file] = byFile[r.file]||[]).push(r); });

  status.textContent = `${results.length} eşleşme, ${Object.keys(byFile).length} dosyada`;

  const q = state.searchCaseSensitive ? query : query.toLowerCase();
  const fragment = document.createDocumentFragment();

  Object.entries(byFile).forEach(([file, matches]) => {
    const group = document.createElement('div');
    group.className = 'search-file-group';
    const header = document.createElement('div');
    header.className = 'search-file-header';
    header.innerHTML = `${fileIconHtml(basename(file),false)}<span>${basename(file)}</span><span class="search-file-count">${matches.length}</span>`;

    const rows = document.createElement('div');
    rows.className = 'search-file-matches';

    matches.forEach(m => {
      const row = document.createElement('div');
      row.className = 'search-match';
      const lineSpan = document.createElement('span');
      lineSpan.className = 'search-match-line';
      lineSpan.textContent = m.line;

      const textSpan = document.createElement('span');
      textSpan.className = 'search-match-text';
      // Highlight the match
      const txt = m.text;
      const idx = (state.searchCaseSensitive ? txt : txt.toLowerCase()).indexOf(q);
      if (idx >= 0) {
        textSpan.innerHTML = escHtml(txt.slice(0,idx)) +
          `<span class="search-match-hl">${escHtml(txt.slice(idx, idx+query.length))}</span>` +
          escHtml(txt.slice(idx+query.length));
      } else {
        textSpan.textContent = txt;
      }

      row.append(lineSpan, textSpan);
      row.addEventListener('click', async () => {
        await openFile(file);
        state.editor.revealLineInCenter(m.line);
        state.editor.setPosition({ lineNumber: m.line, column: m.col });
        state.editor.focus();
      });
      rows.appendChild(row);
    });

    group.append(header, rows);
    fragment.appendChild(group);
  });

  resWrap.appendChild(fragment);
}

function escHtml(s) {
  return s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}
function escapeRegex(s) { return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&'); }

async function replaceAll(query, replacement) {
  if (!query) { notify('Arama terimi boş.', 'info', 2000); return; }
  if (!state.openFolderPath) { notify('Önce bir klasör açın.', 'info', 2000); return; }
  const results = await window.dorukAPI.searchFiles(state.openFolderPath, query, state.searchCaseSensitive);
  if (!results.length) { notify('"' + query + '" bulunamadı.', 'info', 2000); return; }
  const byFile = {};
  results.forEach(r => { (byFile[r.file] = byFile[r.file]||[]).push(r); });
  const flags = state.searchCaseSensitive ? 'g' : 'gi';
  const regex = new RegExp(escapeRegex(query), flags);
  let totalReplaced = 0;
  for (const file of Object.keys(byFile)) {
    const readRes = await window.dorukAPI.fileRead(file);
    if (!readRes.ok) continue;
    const newContent = readRes.content.replace(regex, replacement);
    if (newContent === readRes.content) continue;
    const writeRes = await window.dorukAPI.fileWrite(file, newContent);
    if (!writeRes.ok) continue;
    totalReplaced += byFile[file].length;
    // Açık Monaco modeli varsa güncelle
    const tab = state.tabs.find(t => t.path === file);
    if (tab) { tab.model.setValue(newContent); tab.modified = false; renderTabs(); }
  }
  notify(`✔ ${totalReplaced} eşleşme değiştirildi`, 'success', 2500);
  runSearch(query);
}

// ─── LIVE ANALYSIS ────────────────────────────────────────────────────────────
async function liveAnalyze(filePath) {
  if (!filePath || filePath.startsWith('untitled-') || extname(filePath) !== '.drk') return;
  if (!window.dorukAPI.dorukCheck) return;
  try {
    const output = await window.dorukAPI.dorukCheck(filePath);
    const re = /\[(HATA|UYARI)\] ([^:]+):(\d+):(\d+) → (.+)/g;
    const problems = []; let m;
    while ((m = re.exec(output)) !== null)
      problems.push({ type: m[1]==='HATA'?'error':'warning', file:m[2], line:+m[3], col:+m[4], msg:m[5] });
    const tab = state.tabs.find(t => t.path === filePath);
    if (!tab || !window._monacoReady) return;
    const markers = problems
      .filter(p => tab.path.endsWith(p.file) || basename(tab.path)===p.file)
      .map(p => ({
        severity: p.type==='error' ? monaco.MarkerSeverity.Error : monaco.MarkerSeverity.Warning,
        startLineNumber:p.line, endLineNumber:p.line,
        startColumn:p.col, endColumn:999,
        message:p.msg, source:'doruk-live',
      }));
    monaco.editor.setModelMarkers(tab.model, 'doruk-live', markers);
  } catch {}
}

// ─── OUTLINE ──────────────────────────────────────────────────────────────────
function renderOutline() {
  const emptyEl = $('outline-empty');
  const listEl  = $('outline-list');
  if (!listEl) return;
  const tab = state.tabs[state.activeTabIdx];
  if (!tab) { if (emptyEl) { emptyEl.textContent='Dosya açılmadı.'; emptyEl.style.display=''; } listEl.innerHTML=''; return; }

  const lines = tab.model.getValue().split('\n');
  const items = [];
  const ID = '[a-zA-Z\u00e7\u011f\u0131\u00f6\u015f\u00fc\u00c7\u011e\u0130\u00d6\u015e\u00dc_][a-zA-Z\u00e7\u011f\u0131\u00f6\u015f\u00fc\u00c7\u011e\u0130\u00d6\u015e\u00dc0-9_]*';
  lines.forEach((line, i) => {
    let m;
    if ((m = line.match(new RegExp(`^\\s*fonksiyon\\s+(${ID})\\s*\\(`)))) items.push({ type:'fonksiyon', name:m[1], line:i+1 });
    else if ((m = line.match(new RegExp(`^\\s*s[\\u0131i]n[\\u0131i]f\\s+(${ID})`)))) items.push({ type:'sinif', name:m[1], line:i+1 });
    else if ((m = line.match(new RegExp(`^de[g\\u011f]i[s\\u015f]ken\\s+(${ID})\\s*=`)))) items.push({ type:'degisken', name:m[1], line:i+1 });
    else if ((m = line.match(new RegExp(`^sabit\\s+(${ID})\\s*=`)))) items.push({ type:'sabit', name:m[1], line:i+1 });
  });

  if (!items.length) {
    if (emptyEl) { emptyEl.textContent='Sembol bulunamadı.'; emptyEl.style.display=''; }
    listEl.innerHTML=''; return;
  }
  if (emptyEl) emptyEl.style.display='none';

  const iconMap = { fonksiyon:'ƒ', sinif:'◇', degisken:'○', sabit:'●' };
  listEl.innerHTML = items.map(it =>
    `<div class="outline-item outline-${it.type}" data-line="${it.line}">
      <span class="outline-icon">${iconMap[it.type]||'•'}</span>
      <span class="outline-name">${it.name}</span>
      <span class="outline-line">${it.line}</span>
    </div>`
  ).join('');
  listEl.querySelectorAll('.outline-item').forEach(el => {
    el.addEventListener('click', () => {
      const ln = +el.dataset.line;
      state.editor?.revealLineInCenter(ln);
      state.editor?.setPosition({ lineNumber:ln, column:1 });
      state.editor?.focus();
    });
  });
}

// ─── SESSION RESTORE ──────────────────────────────────────────────────────────
function saveSession() {
  const session = {
    folder: state.openFolderPath,
    tabs: state.tabs.filter(t=>!t.path.startsWith('untitled-')).map(t=>t.path),
    activeTab: state.tabs[state.activeTabIdx]?.path,
  };
  try { localStorage.setItem('msc-session', JSON.stringify(session)); } catch {}
}

async function restoreSession() {
  try {
    const s = JSON.parse(localStorage.getItem('msc-session')||'null');
    if (!s) return;
    if (s.folder) await openFolder(s.folder, true);
    if (s.tabs?.length) {
      for (const p of s.tabs) { try { await openFile(p); } catch {} }
      if (s.activeTab) { const idx=state.tabs.findIndex(t=>t.path===s.activeTab); if(idx>=0) switchTab(idx); }
    }
  } catch {}
}

// ─── DIFF VIEWER ──────────────────────────────────────────────────────────────
let diffEditorInstance = null;
function openDiffView() {
  const tabs = state.tabs.filter(t => !t.path.startsWith('untitled-'));
  if (tabs.length < 2) { notify('Karşılaştırmak için en az 2 dosya açık olmalı.', 'warning'); return; }
  const leftSel  = $('diff-left-sel');
  const rightSel = $('diff-right-sel');
  const opts = tabs.map((t,i) => `<option value="${i}">${t.name}</option>`).join('');
  leftSel.innerHTML = opts;
  rightSel.innerHTML = opts;
  if (tabs.length >= 2) rightSel.value = '1';
  $('diff-overlay').classList.remove('hidden');
  if (!diffEditorInstance) {
    diffEditorInstance = monaco.editor.createDiffEditor($('diff-editor-mount'), {
      theme: 'msc-' + state.currentThemeId,
      fontSize: state.settings.fontSize,
      fontFamily: state.settings.fontFamily,
      readOnly: true,
      renderSideBySide: true,
      enableSplitViewResizing: true,
    });
    window.addEventListener('resize', () => diffEditorInstance?.layout());
  }
  function updateDiff() {
    const l = +leftSel.value, r = +rightSel.value;
    diffEditorInstance.setModel({ original: tabs[l].model, modified: tabs[r].model });
    diffEditorInstance.layout();
  }
  leftSel.onchange  = updateDiff;
  rightSel.onchange = updateDiff;
  updateDiff();
}
function closeDiffView() { $('diff-overlay').classList.add('hidden'); }

// ─── THEME PICKER ─────────────────────────────────────────────────────────────
function openThemePicker() {
  const grid = $('theme-grid');
  grid.innerHTML = '';
  (window.MSC_THEMES || []).forEach(theme => {
    const card = document.createElement('div');
    card.className = 'theme-card' + (theme.id === state.currentThemeId ? ' active' : '');
    card.dataset.themeId = theme.id;

    const m = theme.monaco;
    const previewHtml = `
      <div class="theme-preview" style="background:${m.bg};color:${m.fg}">
        <div><span style="color:${m.kw}">fonksiyon</span> <span style="color:${m.fn}">ana</span><span style="color:#ffd700">()</span> <span style="color:#ffd700">{</span></div>
        <div>&nbsp;&nbsp;<span style="color:${m.kw}">değişken</span> x = <span style="color:${m.num}">42</span>;</div>
        <div>&nbsp;&nbsp;<span style="color:${m.fn}">yazln</span>(<span style="color:${m.str}">"Merhaba!"</span>);</div>
        <div>&nbsp;&nbsp;<span style="color:${m.cmt}">// yorum satırı</span></div>
        <div><span style="color:#ffd700">}</span></div>
      </div>`;

    const dotsHtml = theme.preview.map(c => `<span class="theme-dot" style="background:${c}"></span>`).join('');

    card.innerHTML = `${previewHtml}
      <div class="theme-info" style="background:${m.bg};color:${m.fg}">
        <div>
          <div class="theme-name">${theme.name}</div>
          <div class="theme-desc">${theme.desc}</div>
        </div>
        <div style="display:flex;flex-direction:column;align-items:flex-end;gap:6px">
          <div class="theme-dots">${dotsHtml}</div>
          <button class="theme-apply-btn">Uygula</button>
        </div>
      </div>`;

    card.querySelector('.theme-apply-btn').addEventListener('click', e => {
      e.stopPropagation();
      applyTheme(theme.id);
      $$('.theme-card').forEach(c => c.classList.remove('active'));
      card.classList.add('active');
    });
    card.addEventListener('click', () => card.querySelector('.theme-apply-btn').click());
    grid.appendChild(card);
  });
  $('theme-picker-overlay').classList.remove('hidden');
}

function applyTheme(themeId) {
  state.currentThemeId = themeId;
  if (window.mscApplyTheme) window.mscApplyTheme(themeId);
  notify(`✔ Tema: ${(window.MSC_THEMES||[]).find(t=>t.id===themeId)?.name}`, 'success', 1500);
}

// ─── SETTINGS ─────────────────────────────────────────────────────────────────
function loadSettings() {
  try { Object.assign(state.settings, JSON.parse(localStorage.getItem('msc-settings')||'{}')); } catch {}
}
function saveSettings() {
  try { localStorage.setItem('msc-settings', JSON.stringify(state.settings)); } catch {}
}

function openSettings() {
  $('font-size-val').textContent = state.settings.fontSize;
  $('font-family-sel').value     = state.settings.fontFamily;
  $('tab-size-sel').value        = state.settings.tabSize;
  $('word-wrap-toggle').checked  = state.settings.wordWrap;
  $('line-numbers-toggle').checked = state.settings.lineNumbers;
  $('minimap-toggle').checked    = state.settings.minimap;
  $('bracket-color-toggle').checked = state.settings.bracketColor;
  $('autosave-sel').value        = state.settings.autosave;
  const fos = $('format-on-save-toggle'); if (fos) fos.checked = state.settings.formatOnSave !== false;
  $('settings-overlay').classList.remove('hidden');
}

function applyEditorSettings() {
  if (!state.editor) return;
  state.editor.updateOptions({
    fontSize:     state.settings.fontSize,
    fontFamily:   state.settings.fontFamily,
    tabSize:      state.settings.tabSize,
    wordWrap:     state.settings.wordWrap ? 'on' : 'off',
    lineNumbers:  state.settings.lineNumbers ? 'on' : 'off',
    minimap:      { enabled: state.settings.minimap },
    bracketPairColorization: { enabled: state.settings.bracketColor },
  });
  updateStatusIndent();
  saveSettings();
}

function initSettingsEvents() {
  $('settings-close').addEventListener('click', () => $('settings-overlay').classList.add('hidden'));
  $('settings-overlay').addEventListener('click', e => { if (e.target === $('settings-overlay')) $('settings-overlay').classList.add('hidden'); });

  $$('.sz-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      const delta = +btn.dataset.delta;
      state.settings.fontSize = Math.max(8, Math.min(32, state.settings.fontSize + delta));
      $('font-size-val').textContent = state.settings.fontSize;
      applyEditorSettings();
    });
  });
  $('font-family-sel').addEventListener('change', e => { state.settings.fontFamily = e.target.value; applyEditorSettings(); });
  $('tab-size-sel').addEventListener('change',    e => { state.settings.tabSize    = +e.target.value; applyEditorSettings(); });
  $('word-wrap-toggle').addEventListener('change', e => { state.settings.wordWrap    = e.target.checked; applyEditorSettings(); });
  $('line-numbers-toggle').addEventListener('change', e => { state.settings.lineNumbers  = e.target.checked; applyEditorSettings(); });
  $('minimap-toggle').addEventListener('change',   e => { state.settings.minimap    = e.target.checked; applyEditorSettings(); });
  $('bracket-color-toggle').addEventListener('change', e => { state.settings.bracketColor = e.target.checked; applyEditorSettings(); });
  $('autosave-sel').addEventListener('change',     e => { state.settings.autosave   = e.target.value; saveSettings(); });
  $('format-on-save-toggle')?.addEventListener('change', e => { state.settings.formatOnSave = e.target.checked; saveSettings(); });
  $('open-theme-picker-btn').addEventListener('click', () => { $('settings-overlay').classList.add('hidden'); openThemePicker(); });
}

// ─── TERMINAL ─────────────────────────────────────────────────────────────────
function initTerminal() {
  const out   = $('terminal-output');
  const input = $('terminal-input');
  const btn   = $('terminal-send-btn');
  const history = []; let histIdx = -1;

  function startTerminal() {
    if (state.terminalStarted) return;
    state.terminalStarted = true;
    const cwd = state.openFolderPath || process?.env?.USERPROFILE;
    window.dorukAPI.terminalStart(cwd);
    out.innerHTML = '<span style="color:var(--success-fg)">Terminal başlatıldı. Komut yazın.\r\n</span>';
    window.dorukAPI.onTerminalOut(data => {
      const span = document.createElement('span');
      span.textContent = data;
      out.appendChild(span);
      out.scrollTop = out.scrollHeight;
    });
  }

  function sendCommand() {
    const cmd = input.value;
    if (!cmd.trim()) return;
    if (!state.terminalStarted) startTerminal();
    history.unshift(cmd); histIdx = -1;
    window.dorukAPI.terminalInput(cmd + '\r\n');
    input.value = '';
  }

  btn.addEventListener('click', sendCommand);
  input.addEventListener('keydown', e => {
    if (e.key === 'Enter')      { e.preventDefault(); sendCommand(); }
    if (e.key === 'ArrowUp')    { e.preventDefault(); if (histIdx < history.length-1) { histIdx++; input.value = history[histIdx]; } }
    if (e.key === 'ArrowDown')  { e.preventDefault(); if (histIdx > 0) { histIdx--; input.value = history[histIdx]; } else { histIdx=-1; input.value=''; } }
    if (e.key === 'c' && e.ctrlKey) { window.dorukAPI.terminalInput('\x03'); }
  });

  // Auto-start when terminal tab is selected
  document.addEventListener('terminalTabSelected', startTerminal);
}

// ─── Panel & sidebar ─────────────────────────────────────────────────────────
function switchBottomTab(name) {
  // Eğer panel kapalıysa otomatik aç
  if (!state.panelVisible) {
    state.panelVisible = true;
    $('bottom-panel').style.display = '';
    updatePanelToggleIcon();
    state.editor?.layout();
  }
  state.activeBottomTab = name;
  $$('.panel-tab').forEach(t => t.classList.toggle('active', t.dataset.tab===name));
  $$('.panel-view').forEach(v => v.classList.toggle('active', v.id===`view-${name}`));
  if (name==='terminal') { document.dispatchEvent(new Event('terminalTabSelected')); setTimeout(()=>$('terminal-input')?.focus(),100); }
}

function toggleSidebar() {
  const sidebar = $('sidebar');
  const actBar  = $('activity-bar');
  const visible = sidebar.style.display !== 'none';
  sidebar.style.display = visible ? 'none' : '';
  actBar.style.display  = visible ? 'none' : '';
  state.editor?.layout();
  updateSidebarToggleIcon();
}

function toggleBottomPanel() {
  state.panelVisible = !state.panelVisible;
  $('bottom-panel').style.display = state.panelVisible ? '' : 'none';
  updatePanelToggleIcon();
  state.editor?.layout();
}

function updateSidebarToggleIcon() {
  const btn = $('btn-toggle-sidebar');
  if (!btn) return;
  const visible = $('sidebar').style.display !== 'none';
  btn.style.color = visible ? '' : 'var(--accent)';
}

function updatePanelToggleIcon() {
  const btn = $('btn-toggle-panel');
  if (!btn) return;
  btn.style.color = state.panelVisible ? '' : 'var(--accent)';
}

function switchSidePanel(name) {
  const same = state.activeSidePanel === name;
  state.activeSidePanel = same ? null : name;
  $$('.act-btn').forEach(b => b.classList.toggle('active', b.id===`act-${name}` && !same));
  $$('.sidebar-panel').forEach(p => p.classList.toggle('active', p.id===`panel-${name}` && !same));
}

// ─── Status bar ───────────────────────────────────────────────────────────────
function updateStatusPos(pos)   { $('status-pos').textContent = pos ? `Sat ${pos.lineNumber}, Süt ${pos.column}` : 'Sat 1, Süt 1'; }
function updateStatusLang(path) { $('status-lang').textContent = extname(path)==='.drk' ? 'DORUK' : guessLang(path).toUpperCase(); }
function updateStatusDiag()     { $('status-errors').textContent=`⊗ ${state.errorCount} hata`; $('status-warnings').textContent=`⚠ ${state.warnCount} uyarı`; }
function updateStatusIndent()   {
  const sz = state.settings.tabSize || 4;
  const el = $('status-indent'); if (el) el.textContent = `Boşluk: ${sz}`;
}
function updateStatusSelection(sel, model) {
  const el = $('status-selection'); if (!el) return;
  if (!sel || sel.isEmpty()) { el.style.display='none'; return; }
  const chars = model ? model.getValueInRange(sel).length : 0;
  const lines = sel.endLineNumber - sel.startLineNumber;
  el.textContent = lines > 0 ? `${chars} karakter, ${lines+1} satır seçildi` : `${chars} karakter seçildi`;
  el.style.display = 'flex';
}
function updateTitleBarFile(fileName) {
  const el = $('title-file-name'); if (!el) return;
  el.textContent = fileName ? `${fileName} — Mert Studio Code` : '';
}
function updateBreadcrumb(fp) {
  let d = fp;
  if (state.openFolderPath && fp.startsWith(state.openFolderPath)) d = fp.slice(state.openFolderPath.length).replace(/^[/\\]/,'');
  $('breadcrumb-content').innerHTML = d.replace(/\\/g,'/').split('/').map((p,i,a) =>
    i<a.length-1 ? `<span>${p}</span><span class="sep"> › </span>` : `<span>${p}</span>`
  ).join('');
}

// ─── Context menu ─────────────────────────────────────────────────────────────
function showTreeContextMenu(e, entry) {
  const menu = $('ctx-menu'); menu.classList.remove('hidden');
  const items = entry.isDir
    ? [ { label:'📄 Yeni Dosya',   action:()=>newFileInDir(entry.path) },
        { label:'📁 Yeni Klasör',  action:()=>newFolderInDir(entry.path) },
        { sep:true },
        { label:'📋 Yolu Kopyala', action:()=>navigator.clipboard.writeText(entry.path) },
        { label:'🔍 Gezginde Aç',  action:()=>window.dorukAPI.revealInExplorer(entry.path) } ]
    : [ { label:'📖 Aç',           action:()=>openFile(entry.path) },
        { sep:true },
        { label:'📋 Yolu Kopyala', action:()=>navigator.clipboard.writeText(entry.path) },
        { label:'🔍 Gezginde Aç',  action:()=>window.dorukAPI.revealInExplorer(entry.path) },
        { sep:true },
        { label:'✏️ Yeniden Adlandır', action:()=>renameFile(entry) },
        { label:'🗑 Sil',           action:()=>deleteFile(entry), cls:'ctx-danger' } ];
  menu.innerHTML = items.map(i => i.sep ? '<div class="ctx-sep"></div>' : `<div class="ctx-item ${i.cls||''}">${i.label}</div>`).join('');
  menu.querySelectorAll('.ctx-item').forEach((el,i) => {
    const item = items.filter(x=>!x.sep)[i]; el.addEventListener('click',()=>{ item.action(); hideCtxMenu(); });
  });
  menu.style.left = Math.min(e.clientX, window.innerWidth-200)+'px';
  menu.style.top  = Math.min(e.clientY, window.innerHeight-items.length*32)+'px';
}
function hideCtxMenu() { $('ctx-menu').classList.add('hidden'); }

function showTabContextMenu(e, tabIdx) {
  const menu = $('ctx-menu'); menu.classList.remove('hidden');
  const items = [
    { label:'Kapat',                   action: ()=>closeTab(tabIdx) },
    { label:'Diğerlerini Kapat',       action: ()=>closeOtherTabs(tabIdx) },
    { label:'Sağdakileri Kapat',       action: ()=>closeTabsToRight(tabIdx) },
    { sep: true },
    { label:'Tümünü Kapat',            action: ()=>closeAllTabs() },
    { sep: true },
    { label:'Gezginde Göster',         action: ()=>{ const t=state.tabs[tabIdx]; if(t && !t.path.startsWith('untitled-')) window.dorukAPI.revealInExplorer(t.path); } },
    { label:'Yolu Kopyala',            action: ()=>{ const t=state.tabs[tabIdx]; if(t) navigator.clipboard.writeText(t.path); } },
  ];
  menu.innerHTML = items.map(i => i.sep ? '<div class="ctx-sep"></div>' : `<div class="ctx-item">${i.label}</div>`).join('');
  menu.querySelectorAll('.ctx-item').forEach((el,i) => {
    const item = items.filter(x=>!x.sep)[i]; el.addEventListener('click',()=>{ item.action(); hideCtxMenu(); });
  });
  menu.style.left = Math.min(e.clientX, window.innerWidth-220)+'px';
  menu.style.top  = Math.min(e.clientY, window.innerHeight-items.length*32)+'px';
}

function closeOtherTabs(keepIdx) {
  const keep = state.tabs[keepIdx];
  const toClose = state.tabs.filter((_,i) => i!==keepIdx && !state.tabs[i].modified);
  const modified = state.tabs.filter((_,i) => i!==keepIdx && state.tabs[i].modified);
  if (modified.length && !confirm(`${modified.length} kaydedilmemiş dosya kapatılacak. Devam edilsin mi?`)) return;
  state.tabs.forEach((t,i) => { if (i!==keepIdx) t.model.dispose(); });
  state.tabs = [keep];
  state.activeTabIdx = 0;
  switchTab(0);
}

function closeTabsToRight(fromIdx) {
  const toClose = state.tabs.slice(fromIdx+1);
  const modified = toClose.filter(t => t.modified);
  if (modified.length && !confirm(`${modified.length} kaydedilmemiş dosya kapatılacak. Devam edilsin mi?`)) return;
  toClose.forEach(t => t.model.dispose());
  state.tabs = state.tabs.slice(0, fromIdx+1);
  if (state.activeTabIdx > fromIdx) state.activeTabIdx = fromIdx;
  if (state.tabs.length) switchTab(Math.min(state.activeTabIdx, state.tabs.length-1));
  else { state.activeTabIdx=-1; showWelcomeScreen(); }
  renderTabs();
}

function closeAllTabs() {
  const modified = state.tabs.filter(t => t.modified);
  if (modified.length && !confirm(`${modified.length} kaydedilmemiş dosya kapatılacak. Devam edilsin mi?`)) return;
  state.tabs.forEach(t => t.model.dispose());
  state.tabs = []; state.activeTabIdx = -1;
  showWelcomeScreen(); renderTabs();
}
async function newFileInDir(dir)    { const n=prompt('Yeni dosya adı:'); if(!n)return; const r=await window.dorukAPI.fileNew(dir,n); if(r.ok){await refreshTree();openFile(r.path);}else notify(`Oluşturulamadı: ${r.error}`,'error'); }
async function newFolderInDir(dir)  { const n=prompt('Yeni klasör adı:'); if(!n)return; const r=await window.dorukAPI.dirNew(dir,n); if(r.ok) await refreshTree(); else notify(`Oluşturulamadı: ${r.error}`,'error'); }
async function renameFile(entry)    { const n=prompt('Yeni ad:',entry.name); if(!n||n===entry.name)return; const np=dirname(entry.path)+'/'+n; const r=await window.dorukAPI.fileRename(entry.path,np); if(r.ok){const ti=state.tabs.findIndex(t=>t.path===entry.path); if(ti>=0){state.tabs[ti].path=np;state.tabs[ti].name=n;renderTabs();} await refreshTree();} else notify(`Yeniden adlandırılamadı: ${r.error}`,'error'); }
async function deleteFile(entry)    { if(!confirm(`"${entry.name}" silinsin mi?`))return; const r=await window.dorukAPI.fileDelete(entry.path); if(r.ok){const ti=state.tabs.findIndex(t=>t.path===entry.path);if(ti>=0)closeTab(ti);await refreshTree();notify(`${entry.name} silindi.`,'success',2000);}else notify(`Silinemedi: ${r.error}`,'error'); }

// ─── Command palette ─────────────────────────────────────────────────────────
const COMMANDS = [
  { label:'Dosya Aç',            keybind:'Ctrl+O',         action: openFileDialog },
  { label:'Klasör Aç',           keybind:'Ctrl+K Ctrl+O',  action: openFolderDialog },
  { label:'Yeni Dosya',          keybind:'Ctrl+N',         action: newUntitledFile },
  { label:'Kaydet',              keybind:'Ctrl+S',         action: saveActiveFile },
  { label:'Tümünü Kaydet',       keybind:'',               action: saveAllFiles },
  { label:'Çalıştır',           keybind:'F5',             action: runDoruk },
  { label:'Durdur',              keybind:'Shift+F5',       action: stopDoruk },
  { label:'Sekmeyi Kapat',       keybind:'Ctrl+W',         action: ()=>{ if(state.activeTabIdx>=0) closeTab(state.activeTabIdx); } },
  { label:'Tema Seçici',         keybind:'',               action: openThemePicker },
  { label:'Ayarlar',             keybind:'Ctrl+,',         action: openSettings },
  { label:'Dosya Arama',         keybind:'Ctrl+Shift+F',   action: ()=>switchSidePanel('search') },
  { label:'Semboller',           keybind:'Ctrl+Shift+O',   action: ()=>switchSidePanel('outline') },
  { label:'Dosyaları Karşılaştır', keybind:'',             action: openDiffView },
  { label:'Çıktıyı Temizle',    keybind:'',               action: clearOutput },
  { label:'Sorunlar',            keybind:'Ctrl+Shift+M',   action: ()=>switchBottomTab('problems') },
  { label:'Terminal',            keybind:'Ctrl+`',         action: ()=>switchBottomTab('terminal') },
  { label:'Ağacı Yenile',        keybind:'',               action: refreshTree },
  { label:'Yazı Boyutunu Artır', keybind:'Ctrl++',         action: ()=>{ state.settings.fontSize=Math.min(32,state.settings.fontSize+1); applyEditorSettings(); } },
  { label:'Yazı Boyutunu Azalt', keybind:'Ctrl+-',         action: ()=>{ state.settings.fontSize=Math.max(8,state.settings.fontSize-1); applyEditorSettings(); } },
];

function openCommandPalette(prefill='') {
  $('cmd-palette-overlay').classList.remove('hidden');
  const inp = $('cmd-input'); inp.value = prefill; inp.focus(); filterCommands(prefill);
}
function closeCommandPalette() { $('cmd-palette-overlay').classList.add('hidden'); state.editor?.focus(); }
function filterCommands(q) {
  const clean = q.replace(/^>?\s*/,'').toLowerCase();
  const list  = $('cmd-list');
  const matched = clean ? COMMANDS.filter(c=>c.label.toLowerCase().includes(clean)) : COMMANDS;
  list.innerHTML = matched.map((c,i) => `<div class="cmd-item" data-idx="${i}"><span class="cmd-item-label">${c.label}</span>${c.keybind?`<span class="cmd-item-keybind">${c.keybind}</span>`:''}</div>`).join('');
  list.querySelectorAll('.cmd-item').forEach((el,i) => el.addEventListener('click',()=>{ matched[i].action(); closeCommandPalette(); }));
}

// ─── Dropdown menus ───────────────────────────────────────────────────────────
const MENUS = {
  dosya: [
    { label:'Yeni Dosya',        keybind:'Ctrl+N',  action: newUntitledFile },
    { label:'Dosya Aç...',       keybind:'Ctrl+O',  action: openFileDialog },
    { label:'Klasör Aç...',      keybind:'',        action: openFolderDialog },
    { sep:true },
    { label:'Kaydet',            keybind:'Ctrl+S',  action: saveActiveFile },
    { label:'Farklı Kaydet...',  keybind:'',        action: async()=>{ const t=state.tabs[state.activeTabIdx]; if(t){ const r=await window.dorukAPI.dialogSaveAs(t.model.getValue()); if(r.ok){ t.path=r.path; t.name=basename(r.path); t.modified=false; renderTabs(); notify('Kaydedildi.','success',1800); }}} },
    { label:'Tümünü Kaydet',     keybind:'',        action: saveAllFiles },
    { sep:true },
    { label:'Sekmeyi Kapat',     keybind:'Ctrl+W',  action: ()=>{ if(state.activeTabIdx>=0) closeTab(state.activeTabIdx); } },
  ],
  duzenle: [
    { label:'Geri Al',     keybind:'Ctrl+Z', action:()=>state.editor?.trigger('app','undo',null) },
    { label:'Yeniden Yap', keybind:'Ctrl+Y', action:()=>state.editor?.trigger('app','redo',null) },
    { sep:true },
    { label:'Kes',         keybind:'Ctrl+X', action:()=>state.editor?.trigger('app','editor.action.clipboardCutAction',null) },
    { label:'Kopyala',     keybind:'Ctrl+C', action:()=>state.editor?.trigger('app','editor.action.clipboardCopyAction',null) },
    { sep:true },
    { label:'Bul',         keybind:'Ctrl+F', action:()=>state.editor?.trigger('app','actions.find',null) },
    { label:'Değiştir',    keybind:'Ctrl+H', action:()=>state.editor?.trigger('app','editor.action.startFindReplaceAction',null) },
    { label:'Tümünü Seç', keybind:'Ctrl+A', action:()=>state.editor?.trigger('app','editor.action.selectAll',null) },
    { sep:true },
    { label:'Satırı Yukarı Taşı',  keybind:'Alt+↑', action:()=>state.editor?.trigger('app','editor.action.moveLinesUpAction',null) },
    { label:'Satırı Aşağı Taşı',   keybind:'Alt+↓', action:()=>state.editor?.trigger('app','editor.action.moveLinesDownAction',null) },
    { label:'Satırı Çoğalt',       keybind:'Alt+Shift+↓', action:()=>state.editor?.trigger('app','editor.action.copyLinesDownAction',null) },
    { label:'Seçimi Yoruma Al',     keybind:'Ctrl+/', action:()=>state.editor?.trigger('app','editor.action.commentLine',null) },
  ],
  goruntule: [
    { label:'Dosya Gezgini',  keybind:'Ctrl+Shift+E', action:()=>switchSidePanel('explorer') },
    { label:'Dosya Arama',    keybind:'Ctrl+Shift+F', action:()=>switchSidePanel('search') },
    { label:'Semboller',      keybind:'Ctrl+Shift+O', action:()=>switchSidePanel('outline') },
    { label:'Dosyaları Karşılaştır', keybind:'',      action: openDiffView },
    { sep:true },
    { label:'Çıktı',         keybind:'',             action:()=>switchBottomTab('output') },
    { label:'Sorunlar',       keybind:'Ctrl+Shift+M', action:()=>switchBottomTab('problems') },
    { label:'Terminal',       keybind:'Ctrl+`',       action:()=>switchBottomTab('terminal') },
    { sep:true },
    { label:'Tema Seçici',    keybind:'',             action: openThemePicker },
    { label:'Yakınlaştır',    keybind:'Ctrl++',       action:()=>{ state.settings.fontSize=Math.min(32,state.settings.fontSize+1); applyEditorSettings(); } },
    { label:'Uzaklaştır',    keybind:'Ctrl+-',       action:()=>{ state.settings.fontSize=Math.max(8,state.settings.fontSize-1); applyEditorSettings(); } },
    { label:'Kelime Kaydırma', keybind:'Alt+Z',      action:()=>{ state.settings.wordWrap=!state.settings.wordWrap; applyEditorSettings(); notify(state.settings.wordWrap?'Kelime kaydırma: Açık':'Kelime kaydırma: Kapalı','info',1500); } },
  ],
  calistir: [
    { label:'▶  Çalıştır',    keybind:'F5',        action: runDoruk },
    { label:'■  Durdur',      keybind:'Shift+F5',  action: stopDoruk },
    { sep:true },
    { label:'Çıktıyı Temizle', keybind:'',         action: clearOutput },
  ],
  yardim: [
    { label:'Tema Seçici',      action: openThemePicker },
    { label:'Ayarlar',          action: openSettings },
    { label:'Komut Paleti',     action: ()=>openCommandPalette('>') },
    { sep:true },
    { label:'Hakkında', action: async()=>{ const v=await window.dorukAPI.appVersion(); alert(`Mert Studio Code v${v}\nMonaco Editor tabanlı\nDORUK Programlama Dili IDE`); } },
  ],
};

let openMenu = null;
function showDropdownMenu(name, anchor) {
  if (openMenu===name) { hideDropdown(); return; }
  hideDropdown(); openMenu = name;
  const items = MENUS[name]||[];
  const dd    = $('menu-dropdown');
  dd.innerHTML = items.map(i => i.sep ? '<div class="dd-sep"></div>' : `<div class="dd-item">${i.label}${i.keybind?`<span class="dd-item-keybind">${i.keybind}</span>`:''}</div>`).join('');
  dd.querySelectorAll('.dd-item').forEach((el,i) => { const item=items.filter(x=>!x.sep)[i]; el.addEventListener('click',()=>{ item.action(); hideDropdown(); }); });
  const r = anchor.getBoundingClientRect(); dd.style.left = r.left+'px';
  dd.classList.remove('hidden'); $('menu-overlay').classList.remove('hidden');
}
function hideDropdown() { openMenu=null; $('menu-dropdown').classList.add('hidden'); $('menu-overlay').classList.add('hidden'); }

// ─── Keyboard shortcuts ────────────────────────────────────────────────────────
function initKeyboardShortcuts() {
  document.addEventListener('keydown', e => {
    const ctrl=e.ctrlKey||e.metaKey, shift=e.shiftKey, key=e.key;
    if (ctrl&&shift&&key==='P')     { e.preventDefault(); openCommandPalette('>'); return; }
    if (ctrl&&!shift&&key==='n')    { e.preventDefault(); newUntitledFile(); return; }
    if (ctrl&&!shift&&key==='o')    { e.preventDefault(); openFileDialog(); return; }
    if (ctrl&&!shift&&key==='s')    { e.preventDefault(); saveActiveFile(); return; }
    if (ctrl&&shift&&key==='S')     { e.preventDefault(); saveAllFiles(); return; }
    if (ctrl&&!shift&&key===',')    { e.preventDefault(); openSettings(); return; }
    if (ctrl&&!shift&&key==='w')    { e.preventDefault(); if(state.activeTabIdx>=0) closeTab(state.activeTabIdx); return; }
    if (ctrl&&key==='Tab')          { e.preventDefault(); if(state.tabs.length>1) switchTab((state.activeTabIdx+1)%state.tabs.length); return; }
    if (key==='F5'&&!shift)         { e.preventDefault(); runDoruk(); return; }
    if (key==='F5'&&shift)          { e.preventDefault(); stopDoruk(); return; }
    if (ctrl&&shift&&key==='E')     { e.preventDefault(); switchSidePanel('explorer'); return; }
    if (ctrl&&shift&&key==='F')     { e.preventDefault(); switchSidePanel('search'); setTimeout(()=>$('search-input')?.focus(),100); return; }
    if (ctrl&&!shift&&key==='h')    { e.preventDefault(); if(state.activeSidePanel!=='search') switchSidePanel('search'); const row=$('search-replace-row'); if(row?.classList.contains('hidden')){ row.classList.remove('hidden'); $('btn-toggle-replace')?.classList.add('open'); } setTimeout(()=>$('replace-input')?.focus(),100); return; }
    if (ctrl&&shift&&key==='M')     { e.preventDefault(); switchBottomTab('problems'); return; }
    if (ctrl&&shift&&key==='O')     { e.preventDefault(); switchSidePanel('outline'); return; }
    if (ctrl&&key==='`')            { e.preventDefault(); switchBottomTab('terminal'); return; }
    if (ctrl&&!shift&&key==='b')    { e.preventDefault(); toggleSidebar(); return; }
    if (ctrl&&!shift&&key==='j')    { e.preventDefault(); toggleBottomPanel(); return; }
    if (key==='Escape')             { closeCommandPalette(); hideDropdown(); hideCtxMenu(); closeDiffView(); $('theme-picker-overlay').classList.add('hidden'); $('settings-overlay').classList.add('hidden'); return; }
    if (key==='F12')                { e.preventDefault(); window.dorukAPI.openDevTools?.(); return; }
    if (ctrl&&(key==='='||key==='+')){ e.preventDefault(); state.settings.fontSize=Math.min(32,state.settings.fontSize+1); applyEditorSettings(); }
    if (ctrl&&key==='-')            { e.preventDefault(); state.settings.fontSize=Math.max(8,state.settings.fontSize-1); applyEditorSettings(); }
    if (ctrl&&key==='0')            { e.preventDefault(); state.settings.fontSize=14; applyEditorSettings(); }
  });
}

// ─── Resize ────────────────────────────────────────────────────────────────────
function initResizeHandles() {
  // Sidebar resize
  const sidebar = $('sidebar');
  let resizing=false, startX, startW;
  sidebar.addEventListener('mousedown', e => {
    const r = sidebar.getBoundingClientRect();
    if (Math.abs(e.clientX-r.right) < 6) { resizing=true; startX=e.clientX; startW=sidebar.offsetWidth; document.body.style.cursor='col-resize'; e.preventDefault(); }
  });
  document.addEventListener('mousemove', e => {
    if (!resizing) return;
    const w = Math.max(160, Math.min(500, startW+e.clientX-startX));
    sidebar.style.width = w+'px'; document.documentElement.style.setProperty('--sidebar-width',w+'px');
    state.editor?.layout();
  });
  document.addEventListener('mouseup', () => { resizing=false; document.body.style.cursor=''; });

  // Panel resize — dedicated handle
  const panel      = $('bottom-panel');
  const panelHandle = $('panel-resize-handle');
  let presizing=false, pStartY, pStartH;
  panelHandle.addEventListener('mousedown', e => {
    presizing=true; pStartY=e.clientY; pStartH=panel.offsetHeight;
    document.body.style.cursor='row-resize';
    panelHandle.classList.add('dragging');
    e.preventDefault();
  });
  document.addEventListener('mousemove', e => {
    if (!presizing) return;
    const h = Math.max(80, Math.min(window.innerHeight*0.7, pStartH-(e.clientY-pStartY)));
    panel.style.height=h+'px'; document.documentElement.style.setProperty('--panel-height',h+'px');
    state.editor?.layout();
  });
  document.addEventListener('mouseup', () => {
    if (!presizing) return;
    presizing=false; document.body.style.cursor='';
    panelHandle.classList.remove('dragging');
  });
}

// ─── Exec status ──────────────────────────────────────────────────────────────
async function checkDorukExe() {
  const el = $('doruk-exe-status'); if (!el) return;
  const p  = await window.dorukAPI.dorukPath();
  el.innerHTML = p
    ? `<span style="color:var(--success-fg)">✔ doruk.exe hazır</span><br><small style="color:var(--fg-dim)">${p}</small>`
    : `<span style="color:var(--warning-fg)">⚠ doruk.exe bulunamadı</span><br><small>doruk/derle.bat çalıştırın</small>`;
}

// ─── Init ─────────────────────────────────────────────────────────────────────
async function init() {
  loadRecent(); loadSettings(); renderWelcomeRecent();

  // UI event listenerları Monaco'dan önce kur — böylece Monaco yüklenirken UI tepkisiz kalmaz
  initRunnerEvents();
  initTerminal();
  initKeyboardShortcuts();
  initResizeHandles();
  initSettingsEvents();

  // Window controls
  $('btn-min').addEventListener('click',   ()=>window.dorukAPI.winMinimize());
  $('btn-max').addEventListener('click',   ()=>window.dorukAPI.winMaximize());
  $('btn-close').addEventListener('click', ()=>window.dorukAPI.winClose());
  window.dorukAPI.onWinState(s => { $('btn-max').title = s==='maximized'?'Geri Yükle':'Ekranı Kapla'; });

  // Menu bar
  $$('.menu-item').forEach(el => el.addEventListener('click', ()=>showDropdownMenu(el.dataset.menu, el)));
  $('menu-overlay').addEventListener('click', hideDropdown);

  // Activity bar
  $$('.act-btn[data-panel]').forEach(btn => btn.addEventListener('click', ()=>switchSidePanel(btn.dataset.panel)));
  $('act-settings').addEventListener('click', openSettings);

  // Explorer toolbar
  $('btn-new-file').addEventListener('click',    ()=>state.openFolderPath ? newFileInDir(state.openFolderPath) : newUntitledFile());
  $('btn-new-folder').addEventListener('click',  ()=>state.openFolderPath ? newFolderInDir(state.openFolderPath) : notify('Önce klasör açın','warning'));
  $('btn-open-folder').addEventListener('click',  openFolderDialog);
  $('btn-refresh-tree').addEventListener('click', refreshTree);
  $('btn-open-folder-hint').addEventListener('click', openFolderDialog);
  $('btn-close-folder').addEventListener('click', ()=>{
    state.openFolderPath = null;
    state.treeData = [];
    state.treeExpanded.clear();
    $('folder-label-bar').classList.add('hidden');
    renderFileTree(); // hint'i gösterir, ağacı temizler
  });

  // Welcome
  $('wb-new-file').addEventListener('click',    newUntitledFile);
  $('wb-open-file').addEventListener('click',   openFileDialog);
  $('wb-open-folder').addEventListener('click', openFolderDialog);

  // Run / Stop
  $('btn-run').addEventListener('click',  runDoruk);
  $('btn-stop').addEventListener('click', stopDoruk);
  $('btn-run-big')?.addEventListener('click',  runDoruk);
  $('btn-stop-big')?.addEventListener('click', stopDoruk);

  // Panel controls
  $('btn-clear-output').addEventListener('click', clearOutput);
  $('btn-panel-toggle').addEventListener('click', toggleBottomPanel);
  $('btn-toggle-panel')?.addEventListener('click', toggleBottomPanel);
  $('btn-toggle-sidebar')?.addEventListener('click', toggleSidebar);
  $$('.panel-tab').forEach(t => t.addEventListener('click', ()=>switchBottomTab(t.dataset.tab)));

  // Status bar
  $('status-errors').addEventListener('click',   ()=>switchBottomTab('problems'));
  $('status-warnings').addEventListener('click', ()=>switchBottomTab('problems'));
  $('status-lang').addEventListener('click',     ()=>notify('Dil: DORUK (.drk) — .drk uzantılı dosyalar için', 'info', 2000));
  $('status-indent').addEventListener('click',   ()=>openSettings());
  $('status-pos').addEventListener('click',      ()=>{ const l=prompt('Satıra git:'); if(l&&+l) { state.editor?.revealLineInCenter(+l); state.editor?.setPosition({lineNumber:+l,column:1}); state.editor?.focus(); }});

  // Command palette
  $('cmd-palette-overlay').addEventListener('click', e=>{ if(e.target===$('cmd-palette-overlay')) closeCommandPalette(); });
  $('cmd-input').addEventListener('input',   e=>filterCommands(e.target.value));
  $('cmd-input').addEventListener('keydown', e=>{ if(e.key==='Escape') closeCommandPalette(); if(e.key==='Enter'){const f=$('cmd-list').querySelector('.cmd-item'); if(f) f.click();}});

  // Theme picker
  $('theme-picker-close').addEventListener('click',   ()=>$('theme-picker-overlay').classList.add('hidden'));
  $('theme-picker-overlay').addEventListener('click', e=>{ if(e.target===$('theme-picker-overlay')) $('theme-picker-overlay').classList.add('hidden'); });

  // Search panel
  const searchInput = $('search-input');
  searchInput?.addEventListener('input', e => {
    clearTimeout(searchDebounce);
    searchDebounce = setTimeout(()=>runSearch(e.target.value), 300);
  });
  searchInput?.addEventListener('keydown', e => { if(e.key==='Escape') { searchInput.value=''; $('search-results-wrap').innerHTML=''; $('search-status').textContent=''; }});

  // Search options
  const csBtn = document.createElement('button');
  csBtn.className = 'search-opt-btn'; csBtn.textContent = 'Aa'; csBtn.title = 'Büyük/Küçük harf duyarlı';
  document.querySelector('#search-options')?.appendChild(csBtn);
  csBtn.addEventListener('click', ()=>{ state.searchCaseSensitive=!state.searchCaseSensitive; csBtn.classList.toggle('active',state.searchCaseSensitive); runSearch($('search-input')?.value||''); });

  // Toggle replace row
  $('btn-toggle-replace')?.addEventListener('click', () => {
    const row = $('search-replace-row');
    const btn = $('btn-toggle-replace');
    const hidden = row.classList.toggle('hidden');
    btn.classList.toggle('open', !hidden);
    if (!hidden) $('replace-input')?.focus();
  });

  // Replace enter key shortcut
  $('replace-input')?.addEventListener('keydown', e => {
    if (e.key === 'Enter') replaceAll($('search-input')?.value||'', $('replace-input')?.value||'');
    if (e.key === 'Escape') { $('search-replace-row').classList.add('hidden'); $('btn-toggle-replace')?.classList.remove('open'); }
  });

  // Replace all button
  $('btn-replace-all')?.addEventListener('click', () => {
    replaceAll($('search-input')?.value||'', $('replace-input')?.value||'');
  });

  // Context menu dismiss
  document.addEventListener('click', hideCtxMenu);

  // Split editor button → toggle minimap (practical use instead of dummy)
  $('btn-split-editor')?.addEventListener('click', ()=>{
    state.settings.minimap = !state.settings.minimap;
    applyEditorSettings();
    notify(state.settings.minimap ? 'Minimap: Açık' : 'Minimap: Kapalı', 'info', 1500);
  });

  // Diff viewer close
  $('diff-close')?.addEventListener('click', closeDiffView);

  // Outline list click delegation (for dynamically rendered items)
  $('outline-list')?.addEventListener('click', e => {
    const item = e.target.closest('.outline-item');
    if (!item) return;
    const ln = +item.dataset.line;
    state.editor?.revealLineInCenter(ln);
    state.editor?.setPosition({ lineNumber: ln, column: 1 });
    state.editor?.focus();
  });

  // Save session on unload
  window.addEventListener('beforeunload', saveSession);

  await checkDorukExe();

  // Monaco en son yüklenir — UI zaten tamamen aktif
  await initMonaco();

  // Oturum geri yükle
  await restoreSession();

  console.log('[Mert Studio Code] Hazır!');
  notify('Mert Studio Code hazır!', 'success', 2500);
}

window.addEventListener('DOMContentLoaded', () => {
  init().catch(err => {
    console.error('[MSC] Kritik başlatma hatası:', err);
    // Kırmızı hata ekranı göster
    document.body.insertAdjacentHTML('beforeend',
      `<div id="fatal-error" style="position:fixed;inset:0;background:#1a0000;color:#ff6b6b;
        padding:32px;z-index:99999;font-family:Consolas,monospace;overflow:auto">
        <div style="font-size:20px;font-weight:bold;margin-bottom:16px">⚠ Başlatma Hatası</div>
        <pre style="font-size:13px;white-space:pre-wrap">${err.stack || err.message}</pre>
        <div style="margin-top:24px;font-size:12px;color:#999">
          DevTools açmak için F12'ye basın → Console sekmesinde hatanın tam detayını görebilirsiniz.
        </div>
      </div>`
    );
  });
});

// Global hata yakalayıcı
window.addEventListener('error', e => {
  console.error('[MSC] Script hatası:', e.message, 'Kaynak:', e.filename, 'Satır:', e.lineno);
});
window.addEventListener('unhandledrejection', e => {
  console.error('[MSC] İşlenmeyen Promise reddi:', e.reason);
});
