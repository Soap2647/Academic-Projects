'use strict';

const { contextBridge, ipcRenderer } = require('electron');

// ─── Safe IPC bridge ──────────────────────────────────────────────────────────
contextBridge.exposeInMainWorld('dorukAPI', {

  // Window controls
  winMinimize: ()  => ipcRenderer.send('win-minimize'),
  winMaximize: ()  => ipcRenderer.send('win-maximize'),
  winClose:    ()  => ipcRenderer.send('win-close'),
  onWinState:  (cb) => ipcRenderer.on('window-state', (_, s) => cb(s)),

  // Dialogs
  dialogOpenFile:   ()           => ipcRenderer.invoke('dialog-open-file'),
  dialogOpenFolder: ()           => ipcRenderer.invoke('dialog-open-folder'),
  dialogSaveAs:     (content)    => ipcRenderer.invoke('dialog-save-as', content),

  // File I/O
  fileRead:   (p)        => ipcRenderer.invoke('file-read',   p),
  fileWrite:  (p, c)     => ipcRenderer.invoke('file-write',  p, c),
  fileDelete: (p)        => ipcRenderer.invoke('file-delete', p),
  fileRename: (o, n)     => ipcRenderer.invoke('file-rename', o, n),
  fileNew:    (dir, name) => ipcRenderer.invoke('file-new',   dir, name),
  dirNew:     (dir, name) => ipcRenderer.invoke('dir-new',    dir, name),

  // Directory tree
  dirRead:     (p) => ipcRenderer.invoke('dir-read',      p),
  dirReadDeep: (p) => ipcRenderer.invoke('dir-read-deep', p),

  // Shell
  revealInExplorer: (p) => ipcRenderer.send('reveal-in-explorer', p),

  // DORUK runner
  dorukPath:  ()  => ipcRenderer.invoke('doruk-path'),
  dorukRun:   (p) => ipcRenderer.send('doruk-run',  p),
  dorukStop:  ()  => ipcRenderer.send('doruk-stop'),
  dorukCheck: (p) => ipcRenderer.invoke('doruk-check', p),

  // Runner events (renderer ← main)
  onDorukOutput: (cb) => ipcRenderer.on('doruk-output', (_, d) => cb(d)),
  onDorukExit:   (cb) => ipcRenderer.on('doruk-exit',   (_, d) => cb(d)),
  offDorukOutput: () => ipcRenderer.removeAllListeners('doruk-output'),
  offDorukExit:   () => ipcRenderer.removeAllListeners('doruk-exit'),

  // File search
  searchFiles: (folder, query, cs) => ipcRenderer.invoke('search-files', folder, query, cs),

  // Terminal
  terminalStart:  (cwd) => ipcRenderer.send('terminal-start', cwd),
  terminalInput:  (txt) => ipcRenderer.send('terminal-input', txt),
  terminalStop:   ()    => ipcRenderer.send('terminal-stop'),
  onTerminalOut:  (cb)  => ipcRenderer.on('terminal-output', (_, d) => cb(d)),
  offTerminalOut: ()    => ipcRenderer.removeAllListeners('terminal-output'),

  // App info
  appVersion: () => ipcRenderer.invoke('app-version'),
  platform:   () => ipcRenderer.invoke('platform'),

  // DevTools
  openDevTools: () => ipcRenderer.send('open-devtools'),
});
