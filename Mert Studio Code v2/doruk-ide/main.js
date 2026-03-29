'use strict';

const { app, BrowserWindow, ipcMain, dialog, Menu, shell } = require('electron');
const path  = require('path');
const fs    = require('fs');
const cp    = require('child_process');

// ─── Globals ───────────────────────────────────────────────────────────────────
let mainWindow    = null;
let runningProc   = null;   // doruk.exe child process

// ─── Helper: find doruk.exe ────────────────────────────────────────────────────
function getDorukExePath() {
  const base = path.join(__dirname, '..', 'doruk');
  const candidates = [
    // Paketlenmiş exe yanında (dist/Mert Studio Code-win32-x64/doruk.exe)
    path.join(path.dirname(process.execPath), 'doruk.exe'),
    // Geliştirme modu — kaynak ağacı
    path.join(base, 'doruk.exe'),
    path.join(base, 'build', 'Release', 'doruk.exe'),
    path.join(base, 'build_cli', 'Release', 'doruk.exe'),
    path.join(base, 'build', 'Debug',   'doruk.exe'),
    path.join(base, 'build', 'doruk.exe'),
    path.join(__dirname, 'doruk.exe'),
  ];
  for (const c of candidates) {
    if (fs.existsSync(c)) return c;
  }
  return null;
}

// ─── Window ────────────────────────────────────────────────────────────────────
function createWindow() {
  const iconPath = ['icon_taskbar.png','icon.png','icon.jpg'].map(n=>path.join(__dirname,'assets',n)).find(p=>fs.existsSync(p));
  mainWindow = new BrowserWindow({
    width:           1400,
    height:          900,
    minWidth:        800,
    minHeight:       500,
    frame:           false,          // custom title bar
    transparent:     false,
    backgroundColor: '#1e1e1e',
    icon:            iconPath || undefined,
    webPreferences: {
      preload:          path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration:  false,
      spellcheck:       false,
    },
    show: false,
  });

  mainWindow.loadFile(path.join(__dirname, 'renderer', 'index.html'));
  Menu.setApplicationMenu(null);

  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
    mainWindow.maximize();
  });

  mainWindow.on('maximize',   () => mainWindow.webContents.send('window-state', 'maximized'));
  mainWindow.on('unmaximize', () => mainWindow.webContents.send('window-state', 'normal'));
  mainWindow.on('closed',     () => { mainWindow = null; });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (runningProc) { runningProc.kill(); runningProc = null; }
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});

// ─── Window controls ──────────────────────────────────────────────────────────
ipcMain.on('win-minimize',  () => mainWindow?.minimize());
ipcMain.on('win-maximize',  () => {
  if (mainWindow?.isMaximized()) mainWindow.unmaximize();
  else mainWindow?.maximize();
});
ipcMain.on('win-close', () => mainWindow?.close());

// ─── Dialog: open files ───────────────────────────────────────────────────────
ipcMain.handle('dialog-open-file', async () => {
  const r = await dialog.showOpenDialog(mainWindow, {
    title: 'Dosya Aç',
    filters: [
      { name: 'DORUK Dosyaları', extensions: ['drk'] },
      { name: 'Tüm Dosyalar',    extensions: ['*']   },
    ],
    properties: ['openFile', 'multiSelections'],
  });
  return r.canceled ? [] : r.filePaths;
});

// ─── Dialog: open folder ──────────────────────────────────────────────────────
ipcMain.handle('dialog-open-folder', async () => {
  const r = await dialog.showOpenDialog(mainWindow, {
    title: 'Klasör Aç',
    properties: ['openDirectory'],
  });
  return r.canceled ? null : r.filePaths[0];
});

// ─── Dialog: save as ─────────────────────────────────────────────────────────
ipcMain.handle('dialog-save-as', async (_, content) => {
  const r = await dialog.showSaveDialog(mainWindow, {
    title: 'Farklı Kaydet',
    defaultPath: 'yeni_dosya.drk',
    filters: [
      { name: 'DORUK Dosyaları', extensions: ['drk'] },
      { name: 'Tüm Dosyalar',    extensions: ['*']   },
    ],
  });
  if (r.canceled) return { ok: false };
  try {
    fs.writeFileSync(r.filePath, content, 'utf8');
    return { ok: true, path: r.filePath };
  } catch (e) {
    return { ok: false, error: e.message };
  }
});

// ─── File I/O ─────────────────────────────────────────────────────────────────
ipcMain.handle('file-read', async (_, filePath) => {
  try {
    return { ok: true, content: fs.readFileSync(filePath, 'utf8') };
  } catch (e) {
    return { ok: false, error: e.message };
  }
});

ipcMain.handle('file-write', async (_, filePath, content) => {
  try {
    fs.mkdirSync(path.dirname(filePath), { recursive: true });
    fs.writeFileSync(filePath, content, 'utf8');
    return { ok: true };
  } catch (e) {
    return { ok: false, error: e.message };
  }
});

ipcMain.handle('file-delete', async (_, filePath) => {
  try { fs.unlinkSync(filePath); return { ok: true }; }
  catch (e) { return { ok: false, error: e.message }; }
});

ipcMain.handle('file-rename', async (_, oldPath, newPath) => {
  try { fs.renameSync(oldPath, newPath); return { ok: true }; }
  catch (e) { return { ok: false, error: e.message }; }
});

ipcMain.handle('file-new', async (_, dirPath, name) => {
  const filePath = path.join(dirPath, name);
  try {
    fs.writeFileSync(filePath, '', 'utf8');
    return { ok: true, path: filePath };
  } catch (e) {
    return { ok: false, error: e.message };
  }
});

ipcMain.handle('dir-new', async (_, dirPath, name) => {
  const newDir = path.join(dirPath, name);
  try {
    fs.mkdirSync(newDir, { recursive: true });
    return { ok: true, path: newDir };
  } catch (e) {
    return { ok: false, error: e.message };
  }
});

// ─── Directory tree ───────────────────────────────────────────────────────────
function readDirTree(dirPath, depth = 0) {
  if (depth > 6) return [];
  const SKIP = new Set(['.git', 'node_modules', '__pycache__', '.cache', 'build', 'dist', '.vs']);
  try {
    return fs.readdirSync(dirPath, { withFileTypes: true })
      .filter(e => !e.name.startsWith('.') || e.name === '.gitignore')
      .filter(e => !SKIP.has(e.name))
      .sort((a, b) => {
        if (a.isDirectory() !== b.isDirectory())
          return a.isDirectory() ? -1 : 1;
        return a.name.localeCompare(b.name, 'tr');
      })
      .map(e => ({
        name:      e.name,
        path:      path.join(dirPath, e.name),
        isDir:     e.isDirectory(),
        children:  e.isDirectory() ? null : undefined,   // null = not loaded yet
        ext:       e.isDirectory() ? '' : path.extname(e.name).toLowerCase(),
      }));
  } catch { return []; }
}

ipcMain.handle('dir-read', async (_, dirPath) => readDirTree(dirPath));
ipcMain.handle('dir-read-deep', async (_, dirPath) => {
  function deep(p, d) {
    const entries = readDirTree(p, d);
    return entries.map(e => e.isDir ? { ...e, children: deep(e.path, d + 1) } : e);
  }
  return deep(dirPath, 0);
});

// ─── File search ──────────────────────────────────────────────────────────────
ipcMain.handle('search-files', async (_, folderPath, query, caseSensitive = false) => {
  if (!query || !folderPath) return [];
  const results = [];
  const SKIP = new Set(['.git','node_modules','build','dist','.cache','__pycache__']);

  function walk(dir) {
    try {
      for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
        if (e.name.startsWith('.') || SKIP.has(e.name)) continue;
        const full = path.join(dir, e.name);
        if (e.isDirectory()) walk(full);
        else {
          try {
            const content = fs.readFileSync(full, 'utf8');
            const lines   = content.split('\n');
            const q       = caseSensitive ? query : query.toLowerCase();
            lines.forEach((line, i) => {
              const hay = caseSensitive ? line : line.toLowerCase();
              if (hay.includes(q)) {
                results.push({
                  file: full,
                  name: path.basename(full),
                  line: i + 1,
                  col:  hay.indexOf(q) + 1,
                  text: line.trimEnd().slice(0, 200),
                });
              }
            });
          } catch {}
        }
        if (results.length >= 2000) return;
      }
    } catch {}
  }
  walk(folderPath);
  return results;
});

// ─── Terminal ─────────────────────────────────────────────────────────────────
let terminalProc = null;

ipcMain.on('terminal-start', (event, cwd) => {
  if (terminalProc) { try { terminalProc.kill(); } catch {} terminalProc = null; }
  const shell = process.platform === 'win32' ? 'cmd.exe' : 'bash';
  terminalProc = cp.spawn(shell, [], { cwd: cwd || process.env.USERPROFILE || '/', env: process.env, shell: false });

  terminalProc.stdout.on('data', d => event.sender.send('terminal-output', d.toString()));
  terminalProc.stderr.on('data', d => event.sender.send('terminal-output', d.toString()));
  terminalProc.on('close', () => { terminalProc = null; event.sender.send('terminal-output', '\r\n[oturum kapandı]\r\n'); });
});

ipcMain.on('terminal-input', (_, input) => {
  if (terminalProc && terminalProc.stdin) terminalProc.stdin.write(input);
});

ipcMain.on('terminal-stop', () => {
  if (terminalProc) { try { terminalProc.kill(); } catch {} terminalProc = null; }
});

// ─── Reveal in Explorer ───────────────────────────────────────────────────────
ipcMain.on('reveal-in-explorer', (_, filePath) => {
  shell.showItemInFolder(filePath);
});

// ─── Run DORUK ────────────────────────────────────────────────────────────────
ipcMain.handle('doruk-path', async () => getDorukExePath());

ipcMain.on('doruk-run', (event, filePath) => {
  // Kill previous process if still running
  if (runningProc) {
    try { runningProc.kill('SIGTERM'); } catch {}
    runningProc = null;
  }

  const exe = getDorukExePath();
  if (!exe) {
    event.sender.send('doruk-output', {
      type: 'error',
      text: '❌ doruk.exe bulunamadı!\n   Önce C++ projesini derleyin:\n   cmake .. -G "Visual Studio 18 2026" -A x64\n   cmake --build . --config Release\n',
    });
    event.sender.send('doruk-exit', { code: -1 });
    return;
  }

  const cwd = path.dirname(filePath);
  event.sender.send('doruk-output', {
    type: 'info',
    text: `▶  ${path.basename(filePath)} çalıştırılıyor...\n${'─'.repeat(50)}\n`,
  });

  runningProc = cp.spawn(exe, [filePath], { cwd, env: process.env });

  runningProc.stdout.on('data', d =>
    event.sender.send('doruk-output', { type: 'stdout', text: d.toString() }));
  runningProc.stderr.on('data', d =>
    event.sender.send('doruk-output', { type: 'stderr', text: d.toString() }));
  runningProc.on('error', err =>
    event.sender.send('doruk-output', { type: 'error', text: `Hata: ${err.message}\n` }));
  runningProc.on('close', code => {
    runningProc = null;
    const ok = code === 0;
    event.sender.send('doruk-output', {
      type: ok ? 'info' : 'error',
      text: `${'─'.repeat(50)}\n${ok ? '✔' : '✘'}  Program sonlandı (kod: ${code})\n`,
    });
    event.sender.send('doruk-exit', { code });
  });
});

ipcMain.on('doruk-stop', () => {
  if (runningProc) {
    try { runningProc.kill(); } catch {}
    runningProc = null;
  }
});

// ─── Sessiz sözdizimi denetimi (live diagnostics için) ────────────────────────
ipcMain.handle('doruk-check', async (_, filePath) => {
  const exe = getDorukExePath();
  if (!exe) return '';
  return new Promise(resolve => {
    let output = '';
    const proc = cp.spawn(exe, [filePath], { cwd: path.dirname(filePath), env: process.env });
    proc.stdout.on('data', d => { output += d.toString(); });
    proc.stderr.on('data', d => { output += d.toString(); });
    proc.on('close', () => resolve(output));
    // En fazla 3 saniye bekle, program I/O'da bloke kalabilir
    setTimeout(() => { try { proc.kill(); } catch {} resolve(output); }, 3000);
  });
});

// ─── App info ─────────────────────────────────────────────────────────────────
ipcMain.handle('app-version', async () => app.getVersion());
ipcMain.handle('platform',    async () => process.platform);

// ─── DevTools ─────────────────────────────────────────────────────────────────
ipcMain.on('open-devtools', () => {
  mainWindow?.webContents.openDevTools({ mode: 'detach' });
});

// ─── Renderer crash / error logging ───────────────────────────────────────────
app.on('render-process-gone', (_, wc, details) => {
  console.error('[MSC] Renderer crashed:', details);
});
