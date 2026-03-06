import { useState, useEffect, useRef, useMemo, useCallback } from 'react';
import {
    Play, FileCode2, Terminal, X, Plus, Trash2,
    Files, Search, Upload, Code2,
    SquareTerminal, PanelBottomClose,
    Download, FilePlus, Undo2, Redo2, MousePointerClick,
    PanelLeft, Keyboard, Info, ChevronDown, Languages, Pencil,
    GitGraph, Bug, StepForward, StepBack, PlayCircle, Settings,
    Replace, ChevronRight, Command, FolderPlus, Folder, FolderOpen, Zap, Clock
} from 'lucide-react';
import { themes, LANGUAGES, highlightLine, simulateRun, validateSyntax, detectLanguage, getFileIcon, getSuggestions } from './editorUtils';
import ThemeSelector from './ThemeSelector';
import SettingsModal from './SettingsModal';

const DEFAULT_FILES = [{ id: 1, name: 'program.c.tr', content: LANGUAGES.turkce_c.defaultCode, parentId: null }];
let fileIdCounter = 2;
let folderIdCounter = 1;

export default function TurkceKodEditoru() {
    const [theme, setTheme] = useState('dark');
    const [sidebarOpen, setSidebarOpen] = useState(true);
    const [terminalOpen, setTerminalOpen] = useState(true);
    const [terminalHeight, setTerminalHeight] = useState(180);
    const [consoleOutput, setConsoleOutput] = useState([]);
    const [isRunning, setIsRunning] = useState(false);
    const [activeLine, setActiveLine] = useState(1);
    const [activeCol, setActiveCol] = useState(1);
    const [activeView, setActiveView] = useState('files');
    const [activeMenu, setActiveMenu] = useState(null);
    const [showThemeSelector, setShowThemeSelector] = useState(false);
    const [showSettings, setShowSettings] = useState(false);
    const [showShortcuts, setShowShortcuts] = useState(false);
    const [showAbout, setShowAbout] = useState(false);

    // Find & Replace
    const [showFindReplace, setShowFindReplace] = useState(false);
    const [findText, setFindText] = useState('');
    const [replaceText, setReplaceText] = useState('');
    const [findMatches, setFindMatches] = useState([]);
    const [findIndex, setFindIndex] = useState(0);

    // Modified files tracking
    const [savedContents, setSavedContents] = useState({});

    // Command Palette
    const [showCommandPalette, setShowCommandPalette] = useState(false);
    const [cmdSearch, setCmdSearch] = useState('');

    const [language, setLanguage] = useState('turkce_c');
    const [showLangPicker, setShowLangPicker] = useState(false);
    const [searchQuery, setSearchQuery] = useState('');
    const [renamingFileId, setRenamingFileId] = useState(null);
    const [renameValue, setRenameValue] = useState('');

    // IntelliSense State
    const [suggestions, setSuggestions] = useState([]);
    const [suggestionIndex, setSuggestionIndex] = useState(0);
    const [suggestionPos, setSuggestionPos] = useState({ top: 0, left: 0 });

    // Terminal State
    const [termInput, setTermInput] = useState('');
    const [termHistory, setTermHistory] = useState([]);
    const [termHistoryIndex, setTermHistoryIndex] = useState(-1);

    // Settings State
    const [settings, setSettings] = useState({
        fontSize: 14,
        showLineNumbers: true,
        showMinimap: false,
        autoSave: true,
        wordWrap: false,
        indentGuides: true,
    });

    // Selection Highlight
    const [selectedWord, setSelectedWord] = useState('');

    // File system
    // File system
    const [files, setFiles] = useState(() => JSON.parse(localStorage.getItem('msc_files')) || DEFAULT_FILES);
    const [folders, setFolders] = useState(() => JSON.parse(localStorage.getItem('msc_folders')) || []);
    const [expandedFolders, setExpandedFolders] = useState(() => JSON.parse(localStorage.getItem('msc_expanded')) || {});
    const [openTabs, setOpenTabs] = useState(() => JSON.parse(localStorage.getItem('msc_openTabs')) || [1]);
    const [activeTab, setActiveTab] = useState(() => JSON.parse(localStorage.getItem('msc_activeTab')) || 1);
    const [contextMenu, setContextMenu] = useState(null);

    // Persistence
    useEffect(() => localStorage.setItem('msc_files', JSON.stringify(files)), [files]);
    useEffect(() => localStorage.setItem('msc_folders', JSON.stringify(folders)), [folders]);
    useEffect(() => localStorage.setItem('msc_expanded', JSON.stringify(expandedFolders)), [expandedFolders]);
    useEffect(() => localStorage.setItem('msc_openTabs', JSON.stringify(openTabs)), [openTabs]);
    useEffect(() => localStorage.setItem('msc_activeTab', JSON.stringify(activeTab)), [activeTab]);

    // Undo/Redo per file
    const historyRef = useRef({});
    const skipHistoryRef = useRef(false);

    const textareaRef = useRef(null);
    const highlightRef = useRef(null);
    const lineNumRef = useRef(null);
    const resizeRef = useRef(null);
    const fileInputRef = useRef(null);
    const renameInputRef = useRef(null);

    const t = themes[theme] || themes.dark;
    const activeFile = files.find((f) => f.id === activeTab);
    const code = activeFile?.content || '';
    const lines = code.split('\n');

    // --- Load from LocalStorage ---
    useEffect(() => {
        const saved = localStorage.getItem('mert-studio-v3');
        if (saved) {
            try {
                const data = JSON.parse(saved);
                if (data.theme) setTheme(data.theme);
                if (data.files && data.files.length > 0) {
                    setFiles(data.files);
                    fileIdCounter = Math.max(...data.files.map(f => f.id)) + 1;
                }
                if (data.openTabs) setOpenTabs(data.openTabs);
                if (data.activeTab) setActiveTab(data.activeTab);
                if (data.settings) setSettings(data.settings);
                if (data.language) setLanguage(data.language);
            } catch (e) { console.error('Failed to load state', e); }
        }
    }, []);

    // --- Save to LocalStorage ---
    useEffect(() => {
        if (!settings.autoSave) return;
        const data = { theme, files, openTabs, activeTab, settings, language };
        localStorage.setItem('mert-studio-v3', JSON.stringify(data));
    }, [theme, files, openTabs, activeTab, settings, language]);

    // --- Settings Handler ---
    const updateSettings = (key, value) => {
        setSettings(prev => ({ ...prev, [key]: value }));
    };

    // --- History helpers ---
    const getHistory = (id) => {
        if (!historyRef.current[id]) historyRef.current[id] = { past: [], future: [] };
        return historyRef.current[id];
    };
    const pushHistory = (id, content) => {
        const h = getHistory(id);
        h.past.push(content);
        if (h.past.length > 100) h.past.shift();
        h.future = [];
    };

    // --- Scroll sync ---
    const handleScroll = useCallback(() => {
        const ta = textareaRef.current;
        if (!ta) return;
        if (highlightRef.current) { highlightRef.current.scrollTop = ta.scrollTop; highlightRef.current.scrollLeft = ta.scrollLeft; }
        if (lineNumRef.current) lineNumRef.current.scrollTop = ta.scrollTop;
    }, []);

    // --- Cursor tracking ---
    const handleSelect = useCallback(() => {
        const ta = textareaRef.current;
        if (!ta) return;
        const pos = ta.selectionStart;
        const before = code.slice(0, pos);
        const lb = before.split('\n');
        setActiveLine(lb.length);
        setActiveCol(lb[lb.length - 1].length + 1);
    }, [code]);

    // --- Selection Highlight ---
    const handleSelectWithHighlight = useCallback(() => {
        handleSelect();
        const ta = textareaRef.current;
        if (!ta) return;
        const start = ta.selectionStart;
        const end = ta.selectionEnd;
        if (start !== end) {
            const selected = code.slice(start, end).trim();
            if (selected.length >= 2 && selected.length <= 50 && !/\s/.test(selected)) {
                setSelectedWord(selected);
            } else {
                setSelectedWord('');
            }
        } else {
            // Auto-select word under cursor
            const before = code.slice(0, start);
            const after = code.slice(start);
            const wBefore = before.match(/[a-zA-ZçÇğĞıİöÖşŞüÜ_$0-9]+$/);
            const wAfter = after.match(/^[a-zA-ZçÇğĞıİöÖşŞüÜ_$0-9]+/);
            const word = (wBefore ? wBefore[0] : '') + (wAfter ? wAfter[0] : '');
            setSelectedWord(word.length >= 2 ? word : '');
        }
    }, [code, handleSelect]);

    // --- Keyboard ---
    const handleKeyDown = useCallback((e) => {
        // IntelliSense Navigation
        if (suggestions.length > 0) {
            if (e.key === 'ArrowDown') {
                e.preventDefault();
                setSuggestionIndex(i => (i + 1) % suggestions.length);
                return;
            }
            if (e.key === 'ArrowUp') {
                e.preventDefault();
                setSuggestionIndex(i => (i - 1 + suggestions.length) % suggestions.length);
                return;
            }
            if (e.key === 'Enter' || e.key === 'Tab') {
                e.preventDefault();
                applySuggestion(suggestions[suggestionIndex]);
                return;
            }
            if (e.key === 'Escape') {
                setSuggestions([]);
                return;
            }
        }

        if (e.key === 'Tab') {
            e.preventDefault();
            // Try snippet expansion first
            if (expandSnippet()) return;
            const ta = e.target;
            const start = ta.selectionStart;
            const end = ta.selectionEnd;
            const newCode = code.slice(0, start) + '    ' + code.slice(end);
            updateFileContent(activeTab, newCode);
            requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = start + 4; });
        }
        if (e.key === 'Enter') {
            e.preventDefault();
            const ta = e.target;
            const pos = ta.selectionStart;
            const currentLine = code.slice(0, pos).split('\n').pop();
            const indent = currentLine.match(/^\s*/)[0];
            const endsWithBlock = currentLine.trimEnd().endsWith('{') || currentLine.trimEnd().endsWith(':');
            const extra = endsWithBlock ? '    ' : '';
            const insert = '\n' + indent + extra;
            const newCode = code.slice(0, pos) + insert + code.slice(ta.selectionEnd);
            updateFileContent(activeTab, newCode);
            const np = pos + insert.length;
            requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = np; });
        }
        if (e.ctrlKey && e.key === 'z') { e.preventDefault(); doUndo(); }
        if (e.ctrlKey && e.key === 'y') { e.preventDefault(); doRedo(); }
        if (e.ctrlKey && e.key === ' ') { e.preventDefault(); triggerSuggestions(true); }

        // Alt+Up/Down: Move line
        if (e.altKey && (e.key === 'ArrowUp' || e.key === 'ArrowDown')) {
            e.preventDefault();
            const ta = e.target;
            const pos = ta.selectionStart;
            const cLines = code.split('\n');
            const before = code.slice(0, pos);
            const lineIdx = before.split('\n').length - 1;
            if (e.key === 'ArrowUp' && lineIdx > 0) {
                [cLines[lineIdx - 1], cLines[lineIdx]] = [cLines[lineIdx], cLines[lineIdx - 1]];
                const newCode = cLines.join('\n');
                updateFileContent(activeTab, newCode);
                const newLineStart = cLines.slice(0, lineIdx - 1).join('\n').length + (lineIdx > 1 ? 1 : 0);
                const colOffset = pos - before.lastIndexOf('\n') - 1;
                requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = newLineStart + colOffset; handleSelect(); });
            } else if (e.key === 'ArrowDown' && lineIdx < cLines.length - 1) {
                [cLines[lineIdx], cLines[lineIdx + 1]] = [cLines[lineIdx + 1], cLines[lineIdx]];
                const newCode = cLines.join('\n');
                updateFileContent(activeTab, newCode);
                const newLineStart = cLines.slice(0, lineIdx + 1).join('\n').length + 1;
                const colOffset = pos - before.lastIndexOf('\n') - 1;
                requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = newLineStart + colOffset; handleSelect(); });
            }
            return;
        }

        // Ctrl+D: Duplicate line
        if (e.ctrlKey && e.key === 'd') {
            e.preventDefault();
            const ta = e.target;
            const pos = ta.selectionStart;
            const cLines = code.split('\n');
            const lineIdx = code.slice(0, pos).split('\n').length - 1;
            cLines.splice(lineIdx + 1, 0, cLines[lineIdx]);
            const newCode = cLines.join('\n');
            updateFileContent(activeTab, newCode);
            const newPos = pos + cLines[lineIdx].length + 1;
            requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = newPos; });
            return;
        }

        // Ctrl+Shift+K: Delete line
        if (e.ctrlKey && e.shiftKey && e.key === 'K') {
            e.preventDefault();
            const ta = e.target;
            const pos = ta.selectionStart;
            const cLines = code.split('\n');
            const lineIdx = code.slice(0, pos).split('\n').length - 1;
            if (cLines.length <= 1) { updateFileContent(activeTab, ''); return; }
            cLines.splice(lineIdx, 1);
            const newCode = cLines.join('\n');
            updateFileContent(activeTab, newCode);
            const newPos = Math.min(pos, newCode.length);
            requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = newPos; handleSelect(); });
            return;
        }

        // Ctrl+H: Find & Replace
        if (e.ctrlKey && e.key === 'h') { e.preventDefault(); setShowFindReplace(p => !p); return; }
        // Ctrl+F: Find (also opens find bar)
        if (e.ctrlKey && e.key === 'f') { e.preventDefault(); setShowFindReplace(true); return; }
        // Ctrl+Shift+P: Command Palette
        if (e.ctrlKey && e.shiftKey && e.key === 'P') { e.preventDefault(); setShowCommandPalette(true); setCmdSearch(''); return; }
    }, [code, activeTab, suggestions, suggestionIndex]);

    // --- IntelliSense Logic ---
    const triggerSuggestions = (force = false) => {
        const ta = textareaRef.current;
        if (!ta) return;
        const pos = ta.selectionStart;
        const textBefore = code.slice(0, pos);
        const match = textBefore.match(/([a-zA-ZçÇğĞıİöÖşŞüÜ_$][a-zA-Z0-9çÇğĞıİöÖşŞüÜ_$]*)$/);

        if (match) {
            const prefix = match[1];
            if (prefix.length >= 1 || force) {
                const sugs = getSuggestions(language, prefix);
                if (sugs.length > 0) {
                    setSuggestions(sugs);
                    setSuggestionIndex(0);
                    // Calculate position (approximation)
                    const lines = textBefore.split('\n');
                    const lineIndex = lines.length - 1;
                    const colIndex = lines[lineIndex].length - prefix.length;

                    // 20px line height, 8px char width (approx for monospace)
                    const top = (lineIndex + 1) * 20;
                    const left = colIndex * 8.4 + 50; // +50 for line numbers
                    setSuggestionPos({ top, left });
                    return;
                }
            }
        }
        setSuggestions([]);
    };

    const applySuggestion = (s) => {
        if (!s) return;
        const ta = textareaRef.current;
        const pos = ta.selectionStart;
        const textBefore = code.slice(0, pos);
        const match = textBefore.match(/([a-zA-ZçÇğĞıİöÖşŞüÜ_$][a-zA-Z0-9çÇğĞıİöÖşŞüÜ_$]*)$/);
        if (match) {
            const prefix = match[1];
            const newCode = code.slice(0, pos - prefix.length) + s.text + code.slice(pos);
            updateFileContent(activeTab, newCode);
            const newPos = pos - prefix.length + s.text.length;
            requestAnimationFrame(() => {
                ta.selectionStart = ta.selectionEnd = newPos;
                ta.focus();
            });
        }
        setSuggestions([]);
    };

    const handleKeyUp = (e) => {
        if (['ArrowUp', 'ArrowDown', 'Enter', 'Tab', 'Escape'].includes(e.key)) return;
        handleSelect();
        triggerSuggestions();
    };

    // --- Find & Replace Logic ---
    useEffect(() => {
        if (!findText || findText.length < 1) { setFindMatches([]); setFindIndex(0); return; }
        const matches = [];
        let idx = 0;
        const lower = code.toLowerCase();
        const searchLower = findText.toLowerCase();
        while (idx < code.length) {
            const found = lower.indexOf(searchLower, idx);
            if (found === -1) break;
            matches.push(found);
            idx = found + 1;
        }
        setFindMatches(matches);
        setFindIndex(0);
    }, [findText, code]);

    const findNext = () => {
        if (findMatches.length === 0) return;
        const next = (findIndex + 1) % findMatches.length;
        setFindIndex(next);
        const ta = textareaRef.current;
        if (ta) {
            ta.focus();
            ta.selectionStart = findMatches[next];
            ta.selectionEnd = findMatches[next] + findText.length;
        }
    };
    const findPrev = () => {
        if (findMatches.length === 0) return;
        const prev = (findIndex - 1 + findMatches.length) % findMatches.length;
        setFindIndex(prev);
        const ta = textareaRef.current;
        if (ta) {
            ta.focus();
            ta.selectionStart = findMatches[prev];
            ta.selectionEnd = findMatches[prev] + findText.length;
        }
    };
    const replaceOne = () => {
        if (findMatches.length === 0 || !findText) return;
        const pos = findMatches[findIndex];
        const newCode = code.slice(0, pos) + replaceText + code.slice(pos + findText.length);
        updateFileContent(activeTab, newCode);
    };
    const replaceAll = () => {
        if (!findText) return;
        const newCode = code.split(findText).join(replaceText);
        updateFileContent(activeTab, newCode);
    };

    // --- Bracket Matching ---
    const matchedBrackets = useMemo(() => {
        const ta = textareaRef.current;
        if (!ta || !code) return null;
        const pos = ta.selectionStart;
        const pairs = { '{': '}', '(': ')', '[': ']' };
        const closers = { '}': '{', ')': '(', ']': '[' };
        const ch = code[pos];
        const prevCh = pos > 0 ? code[pos - 1] : '';

        const findMatch = (startPos, openCh, closeCh, dir) => {
            let depth = 0;
            for (let i = startPos; dir > 0 ? i < code.length : i >= 0; i += dir) {
                if (code[i] === openCh) depth++;
                if (code[i] === closeCh) depth--;
                if (depth === 0) return i;
            }
            return -1;
        };

        if (pairs[ch]) {
            const match = findMatch(pos, ch, pairs[ch], 1);
            if (match !== -1) return { open: pos, close: match };
        }
        if (closers[ch]) {
            const match = findMatch(pos, ch, closers[ch], -1);
            if (match !== -1) return { open: match, close: pos };
        }
        if (pairs[prevCh]) {
            const match = findMatch(pos - 1, prevCh, pairs[prevCh], 1);
            if (match !== -1) return { open: pos - 1, close: match };
        }
        if (closers[prevCh]) {
            const match = findMatch(pos - 1, prevCh, closers[prevCh], -1);
            if (match !== -1) return { open: match, close: pos - 1 };
        }
        return null;
    }, [code, activeLine, activeCol]);

    // --- Modified Files Tracking ---
    useEffect(() => {
        if (Object.keys(savedContents).length === 0 && files.length > 0) {
            const initial = {};
            files.forEach(f => { initial[f.id] = f.content; });
            setSavedContents(initial);
        }
    }, []);

    const isFileModified = (fileId) => {
        const f = files.find(fi => fi.id === fileId);
        if (!f) return false;
        return savedContents[fileId] !== undefined && savedContents[fileId] !== f.content;
    };

    // --- Snippet System ---
    const snippets = useMemo(() => ({
        'if': 'eğer (koşul) {\n    // kod\n}',
        'ife': 'eğer (koşul) {\n    // kod\n} değilse {\n    // alternatif\n}',
        'for': 'döngü (i = 0; i < N; i++) {\n    // kod\n}',
        'while': 'iken (koşul) {\n    // kod\n}',
        'func': 'fonksiyon isim() {\n    // kod\n    döndür;\n}',
        'class': 'sınıf İsim {\n    yapıcı() {\n    }\n}',
        'try': 'dene {\n    // kod\n} yakala (hata) {\n    // hata işleme\n}',
        'switch': 'seç (değer) {\n    durum 1:\n        // kod\n        kır;\n    varsayılan:\n        // kod\n}',
        'main': 'fonksiyon ana() {\n    yazdır("Merhaba Dünya!");\n}',
        'log': 'yazdır();',
        'clog': 'console.log();',
        'arr': 'const dizi = [];',
        'obj': 'const nesne = {};',
    }), []);

    const expandSnippet = useCallback(() => {
        const ta = textareaRef.current;
        if (!ta) return false;
        const pos = ta.selectionStart;
        const before = code.slice(0, pos);
        const match = before.match(/([a-zA-Z]+)$/);
        if (!match) return false;
        const trigger = match[1].toLowerCase();
        if (!snippets[trigger]) return false;
        const snippet = snippets[trigger];
        const newCode = code.slice(0, pos - trigger.length) + snippet + code.slice(pos);
        updateFileContent(activeTab, newCode);
        const newPos = pos - trigger.length + snippet.length;
        requestAnimationFrame(() => { ta.selectionStart = ta.selectionEnd = newPos; });
        return true;
    }, [code, activeTab, snippets]);

    // --- Drag & Drop ---
    const handleDrop = useCallback((e) => {
        e.preventDefault();
        e.stopPropagation();
        const droppedFiles = e.dataTransfer?.files;
        if (!droppedFiles || droppedFiles.length === 0) return;
        Array.from(droppedFiles).forEach(file => {
            const reader = new FileReader();
            reader.onload = (ev) => {
                const content = ev.target.result;
                const newId = fileIdCounter++;
                const newFile = { id: newId, name: file.name, content };
                setFiles(prev => [...prev, newFile]);
                setOpenTabs(prev => [...prev, newId]);
                setActiveTab(newId);
                const detected = detectLanguage(file.name);
                if (detected) setLanguage(detected);
                setSavedContents(prev => ({ ...prev, [newId]: content }));
            };
            reader.readAsText(file);
        });
    }, []);

    const handleDragOver = useCallback((e) => { e.preventDefault(); e.stopPropagation(); }, []);

    // --- Command Palette ---
    const commands = useMemo(() => [
        { label: 'Yeni Dosya', shortcut: '', action: () => { createNewFile(); setShowCommandPalette(false); }, icon: '📄' },
        { label: 'Dosya Aç', shortcut: '', action: () => { fileInputRef.current?.click(); setShowCommandPalette(false); }, icon: '📂' },
        { label: 'Ayarları Aç', shortcut: '', action: () => { setShowSettings(true); setShowCommandPalette(false); }, icon: '⚙️' },
        { label: 'Tema Seçici', shortcut: '', action: () => { setShowThemeSelector(true); setShowCommandPalette(false); }, icon: '🎨' },
        { label: 'Bul & Değiştir', shortcut: 'Ctrl+H', action: () => { setShowFindReplace(true); setShowCommandPalette(false); }, icon: '🔍' },
        { label: 'Terminali Aç/Kapat', shortcut: '', action: () => { setTerminalOpen(p => !p); setShowCommandPalette(false); }, icon: '💻' },
        { label: 'Kenar Çubuğu Aç/Kapat', shortcut: '', action: () => { setSidebarOpen(p => !p); setShowCommandPalette(false); }, icon: '📌' },
        { label: 'Çalıştır', shortcut: '', action: () => { handleRun(); setShowCommandPalette(false); }, icon: '▶️' },
        { label: 'Geri Al', shortcut: 'Ctrl+Z', action: () => { doUndo(); setShowCommandPalette(false); }, icon: '↩️' },
        { label: 'İleri Al', shortcut: 'Ctrl+Y', action: () => { doRedo(); setShowCommandPalette(false); }, icon: '↪️' },
        { label: 'Hakkında', shortcut: '', action: () => { setShowAbout(true); setShowCommandPalette(false); }, icon: 'ℹ️' },
        { label: 'Klavye Kısayolları', shortcut: '', action: () => { setShowShortcuts(true); setShowCommandPalette(false); }, icon: '⌨️' },
        { label: 'Tüm Sekmeleri Kapat', shortcut: '', action: () => { closeAllTabs(); setShowCommandPalette(false); }, icon: '✖️' },
        { label: 'Dosya İndir', shortcut: '', action: () => { downloadFile(); setShowCommandPalette(false); }, icon: '💾' },
    ], []);

    const filteredCommands = useMemo(() => {
        if (!cmdSearch) return commands;
        const q = cmdSearch.toLowerCase();
        return commands.filter(c => c.label.toLowerCase().includes(q));
    }, [cmdSearch, commands]);

    // --- Terminal Logic ---
    const handleTerminalCmd = (e) => {
        if (e.key === 'ArrowUp') {
            e.preventDefault();
            if (termHistory.length > 0) {
                const newIdx = Math.min(termHistoryIndex + 1, termHistory.length - 1);
                setTermHistoryIndex(newIdx);
                setTermInput(termHistory[termHistory.length - 1 - newIdx]);
            }
            return;
        }
        if (e.key === 'ArrowDown') {
            e.preventDefault();
            if (termHistoryIndex > 0) {
                const newIdx = termHistoryIndex - 1;
                setTermHistoryIndex(newIdx);
                setTermInput(termHistory[termHistory.length - 1 - newIdx]);
            } else {
                setTermHistoryIndex(-1);
                setTermInput('');
            }
            return;
        }
        if (e.key === 'Enter') {
            const cmd = termInput.trim();
            if (!cmd) return;
            setTermHistory(p => [...p, cmd]);
            setTermHistoryIndex(-1);
            setTermInput('');

            setConsoleOutput(p => [...p, { type: 'command', text: `> ${cmd}` }]);

            const parts = cmd.split(' ');
            const main = parts[0].toLowerCase();
            const arg = parts[1];

            switch (main) {
                case 'yardim':
                case 'help':
                    setConsoleOutput(p => [...p,
                    { type: 'info', text: 'Kullanılabilir komutlar:' },
                    { type: 'output', text: '  ls              - Dosyaları listele' },
                    { type: 'output', text: '  temizle / clear - Terminali temizle' },
                    { type: 'output', text: '  dosya [isim]    - Yeni dosya oluştur' },
                    { type: 'output', text: '  sil [isim]      - Dosyayı sil' },
                    { type: 'output', text: '  tarih           - Şu anki zamanı göster' },
                    ]);
                    break;
                case 'temizle':
                case 'clear':
                    setConsoleOutput([]);
                    break;
                case 'ls':
                    setConsoleOutput(p => [...p, { type: 'output', text: files.map(f => f.name).join('  ') }]);
                    break;
                case 'dosya':
                    if (arg) {
                        const id = ++fileIdCounter;
                        setFiles(prev => [...prev, { id, name: arg, content: `// ${arg}\n` }]);
                        setConsoleOutput(p => [...p, { type: 'success', text: `Dosya oluşturuldu: ${arg}` }]);
                    } else {
                        setConsoleOutput(p => [...p, { type: 'error', text: 'Hata: Dosya adı belirtmediniz.' }]);
                    }
                    break;
                case 'sil':
                    if (arg) {
                        const f = files.find(f => f.name === arg);
                        if (f) {
                            deleteFile(f.id);
                            setConsoleOutput(p => [...p, { type: 'success', text: `Dosya silindi: ${arg}` }]);
                        } else {
                            setConsoleOutput(p => [...p, { type: 'error', text: `Hata: '${arg}' bulunamadı.` }]);
                        }
                    } else {
                        setConsoleOutput(p => [...p, { type: 'error', text: 'Hata: Silinecek dosya adını belirtmediniz.' }]);
                    }
                    break;
                case 'tarih':
                    setConsoleOutput(p => [...p, { type: 'output', text: new Date().toLocaleString('tr-TR') }]);
                    break;
                default:
                    setConsoleOutput(p => [...p, { type: 'error', text: `Komut bulunamadı: ${main}` }]);
            }
            setTimeout(() => {
                const body = document.querySelector('.ide-terminal-body');
                if (body) body.scrollTop = body.scrollHeight;
            }, 10);
        }
    };

    // --- File operations ---
    const updateFileContent = (id, content) => {
        const prev = files.find((f) => f.id === id)?.content;
        if (prev !== undefined && !skipHistoryRef.current) pushHistory(id, prev);
        setFiles((fs) => fs.map((f) => (f.id === id ? { ...f, content } : f)));
    };

    const doUndo = () => {
        if (!activeTab) return;
        const h = getHistory(activeTab);
        if (h.past.length === 0) return;
        const prev = h.past.pop();
        h.future.push(code);
        skipHistoryRef.current = true;
        setFiles((fs) => fs.map((f) => (f.id === activeTab ? { ...f, content: prev } : f)));
        skipHistoryRef.current = false;
    };
    const doRedo = () => {
        if (!activeTab) return;
        const h = getHistory(activeTab);
        if (h.future.length === 0) return;
        const next = h.future.pop();
        h.past.push(code);
        skipHistoryRef.current = true;
        setFiles((fs) => fs.map((f) => (f.id === activeTab ? { ...f, content: next } : f)));
        skipHistoryRef.current = false;
    };

    const createNewFile = (parentId = null) => {
        const id = ++fileIdCounter;
        const ext = language === 'turkce_python' ? '.py.tr' : (LANGUAGES[language]?.extensions?.[0] || '.c.tr');
        const comment = LANGUAGES[language]?.commentChar || '//';
        const name = `dosya_${id}${ext}`;
        setFiles((prev) => [...prev, { id, name, content: `${comment} ${name}\n`, parentId }]);
        setOpenTabs((prev) => [...prev, id]);
        setActiveTab(id);
    };

    const createFolder = (parentId = null) => {
        const id = ++folderIdCounter;
        const name = `yeni_klasör_${id}`;
        setFolders(prev => [...prev, { id, name, parentId }]);
        setExpandedFolders(prev => ({ ...prev, [id]: true }));
    };

    const toggleFolder = (folderId) => {
        setExpandedFolders(prev => ({ ...prev, [folderId]: !prev[folderId] }));
    };

    const deleteFolder = (folderId) => {
        // Recursively delete all children
        const childFolderIds = folders.filter(f => f.parentId === folderId).map(f => f.id);
        childFolderIds.forEach(cfId => deleteFolder(cfId));
        // Delete files in this folder
        const childFileIds = files.filter(f => f.parentId === folderId).map(f => f.id);
        childFileIds.forEach(id => closeTab(id));
        setFiles(prev => prev.filter(f => f.parentId !== folderId));
        setFolders(prev => prev.filter(f => f.id !== folderId));
    };

    const renameFolder = (folderId, newName) => {
        setFolders(prev => prev.map(f => f.id === folderId ? { ...f, name: newName } : f));
    };

    const openFile = (id) => {
        if (!openTabs.includes(id)) setOpenTabs((prev) => [...prev, id]);
        setActiveTab(id);
    };
    const closeTab = (id, e) => {
        e?.stopPropagation();
        const newTabs = openTabs.filter((tid) => tid !== id);
        setOpenTabs(newTabs);
        if (activeTab === id) setActiveTab(newTabs[newTabs.length - 1] || null);
    };
    const closeAllTabs = () => { setOpenTabs([]); setActiveTab(null); };

    const deleteFile = (id) => {
        if (files.length <= 1) return;
        setFiles((prev) => prev.filter((f) => f.id !== id));
        closeTab(id);
        setContextMenu(null);
    };

    // --- Rename file ---
    const startRename = (id) => {
        const f = files.find(fi => fi.id === id);
        if (f) { setRenamingFileId(id); setRenameValue(f.name); }
    };
    const commitRename = () => {
        if (renamingFileId && renameValue.trim()) {
            setFiles(fs => fs.map(f => f.id === renamingFileId ? { ...f, name: renameValue.trim() } : f));
        }
        setRenamingFileId(null);
        setRenameValue('');
    };

    const handleFileUpload = (e) => {
        const file = e.target.files?.[0];
        if (!file) return;
        const reader = new FileReader();
        reader.onload = (ev) => {
            const id = ++fileIdCounter;
            setFiles((prev) => [...prev, { id, name: file.name, content: ev.target.result }]);
            setOpenTabs((prev) => [...prev, id]);
            setActiveTab(id);
        };
        reader.readAsText(file);
        e.target.value = '';
    };
    const downloadFile = () => {
        if (!activeFile) return;
        const blob = new Blob([code], { type: 'text/plain' });
        const a = document.createElement('a');
        a.href = URL.createObjectURL(blob);
        a.download = activeFile.name;
        a.click();
        URL.revokeObjectURL(a.href);
    };
    const selectAll = () => { if (textareaRef.current) { textareaRef.current.focus(); textareaRef.current.select(); } };

    // --- Auto-detect language from active file ---
    useEffect(() => {
        if (activeFile) setLanguage(detectLanguage(activeFile.name));
    }, [activeTab, activeFile?.name]);

    // Focus rename input
    useEffect(() => {
        if (renamingFileId && renameInputRef.current) {
            renameInputRef.current.focus();
            renameInputRef.current.select();
        }
    }, [renamingFileId]);

    // --- Run with validation ---
    const handleRun = useCallback(() => {
        setIsRunning(true);
        setTerminalOpen(true);
        setTimeout(() => {
            const errors = validateSyntax(code, language);
            const realErrors = errors.filter((e) => e.severity !== 'warning');
            const warnings = errors.filter((e) => e.severity === 'warning');
            if (realErrors.length > 0) {
                setConsoleOutput([
                    { type: 'info', text: '▶ Derleme başlatılıyor...' },
                    ...realErrors.map((e) => ({ type: 'error', text: `✗ Hata (satır ${e.line}): ${e.message}` })),
                    ...warnings.map((e) => ({ type: 'warning', text: `⚠ Uyarı (satır ${e.line}): ${e.message}` })),
                    { type: 'error', text: `\n✗ Derleme başarısız — ${realErrors.length} hata bulundu.` },
                ]);
                setIsRunning(false);
                return;
            }
            const output = simulateRun(code, language);
            const hasRuntimeError = output.some(line => line.startsWith('❌'));

            setConsoleOutput(prev => [
                // Keep previous output? No, usually we clear or separate. Let's just append for history or clear? 
                // The current logic replaces previous output in the setConsoleOutput call below (effectively clearing)
                // actually the code uses setConsoleOutput([...]) which replaces it? 
                // No, previously it was setConsoleOutput([...]). 
                // Let's keep it replacing to simulate a fresh run.
                { type: 'info', text: `▶ ${LANGUAGES[language]?.name || 'Program'} çalıştırılıyor...` },
                ...warnings.map((e) => ({ type: 'warning', text: `⚠ Uyarı (satır ${e.line}): ${e.message}` })),
                ...output.map((line) => ({
                    type: line.startsWith('❌') ? 'error' : 'output',
                    text: line
                })),
                ...(hasRuntimeError
                    ? [{ type: 'error', text: '🛑 Program hata nedeniyle durduruldu.' }]
                    : [{ type: 'success', text: '✓ Program başarıyla tamamlandı.' }]
                ),
            ]);
            setIsRunning(false);
        }, 400);
    }, [code, language]);

    // --- Terminal resize ---
    useEffect(() => {
        const onMove = (e) => {
            if (!resizeRef.current) return;
            const c = document.querySelector('.ide-main');
            if (!c) return;
            const rect = c.getBoundingClientRect();
            setTerminalHeight(Math.max(100, Math.min(rect.bottom - e.clientY, rect.height - 100)));
        };
        const onUp = () => { resizeRef.current = false; document.body.style.cursor = ''; };
        window.addEventListener('mousemove', onMove);
        window.addEventListener('mouseup', onUp);
        return () => { window.removeEventListener('mousemove', onMove); window.removeEventListener('mouseup', onUp); };
    }, []);

    // Close menus on click
    useEffect(() => {
        const close = () => { setContextMenu(null); setActiveMenu(null); setShowLangPicker(false); };
        window.addEventListener('click', close);
        return () => window.removeEventListener('click', close);
    }, []);

    // --- Highlighted HTML ---
    const highlightedHtml = useMemo(
        () => lines.map((l) => highlightLine(l, theme, language)).join('\n'),
        [code, theme, language]
    );

    // --- Search results ---
    const searchResults = useMemo(() => {
        if (!searchQuery || searchQuery.length < 2) return [];
        const results = [];
        const q = searchQuery.toLowerCase();
        for (const f of files) {
            const fLines = f.content.split('\n');
            for (let i = 0; i < fLines.length; i++) {
                if (fLines[i].toLowerCase().includes(q)) {
                    results.push({ fileId: f.id, fileName: f.name, line: i + 1, text: fLines[i].trim() });
                    if (results.length >= 50) return results;
                }
            }
        }
        return results;
    }, [searchQuery, files]);

    // ============ MENU DEFINITIONS ============
    const menuDefs = {
        Dosya: [
            { label: 'Yeni Dosya', icon: <FilePlus size={14} />, action: createNewFile },
            { label: 'Dosya Aç...', icon: <Upload size={14} />, action: () => fileInputRef.current?.click() },
            { label: 'İndir', icon: <Download size={14} />, action: downloadFile, disabled: !activeFile },
            { divider: true },
            { label: 'Ayarlar', icon: <Settings size={14} />, action: () => setShowSettings(true) },
            { divider: true },
            { label: 'Tüm Sekmeleri Kapat', icon: <X size={14} />, action: closeAllTabs },
        ],
        Düzen: [
            { label: 'Geri Al', icon: <Undo2 size={14} />, shortcut: 'Ctrl+Z', action: doUndo },
            { label: 'İleri Al', icon: <Redo2 size={14} />, shortcut: 'Ctrl+Y', action: doRedo },
            { divider: true },
            { label: 'Tümünü Seç', icon: <MousePointerClick size={14} />, shortcut: 'Ctrl+A', action: selectAll },
        ],
        Görünüm: [
            { label: sidebarOpen ? 'Kenar Çubuğunu Gizle' : 'Kenar Çubuğunu Göster', icon: <PanelLeft size={14} />, action: () => setSidebarOpen((p) => !p) },
            { label: terminalOpen ? 'Terminali Gizle' : 'Terminali Göster', icon: <SquareTerminal size={14} />, action: () => setTerminalOpen((p) => !p) },
            { divider: true },
            { label: 'Tema Seçici', icon: <span style={{ fontSize: 14 }}>🎨</span>, action: () => setShowThemeSelector(true) },
        ],
        Yardım: [
            { label: 'Klavye Kısayolları', icon: <Keyboard size={14} />, action: () => setShowShortcuts(true) },
            { divider: true },
            { label: 'Hakkında', icon: <Info size={14} />, action: () => setShowAbout(true) },
        ],
    };

    const fileInput = <input ref={fileInputRef} type="file" accept=".tr,.txt,.c,.h,.js,.jsx,.ts,.tsx,.py,.java,.cpp,.hpp,.cs,.go,.rs,.rb,.php,.html,.htm,.css" onChange={handleFileUpload} style={{ display: 'none' }} />;

    return (
        <div className={`ide-shell theme-${theme}`} onDrop={handleDrop} onDragOver={handleDragOver}>
            {fileInput}

            {/* ===== TITLE BAR ===== */}
            <div className="ide-titlebar transition-theme" style={{ background: t.headerBg, borderBottom: `1px solid ${t.headerBorder}` }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
                    <Code2 size={18} style={{ color: t.keyword }} />
                    <span style={{ color: t.textColor, fontWeight: 700, fontSize: 13 }}>Mert Studio Code</span>
                    <div style={{ display: 'flex', gap: 0, marginLeft: 12, position: 'relative' }}>
                        {Object.keys(menuDefs).map((menuName) => (
                            <div key={menuName} style={{ position: 'relative' }}>
                                <button
                                    onClick={(e) => { e.stopPropagation(); setActiveMenu((p) => p === menuName ? null : menuName); }}
                                    onMouseEnter={() => { if (activeMenu) setActiveMenu(menuName); }}
                                    style={{
                                        background: activeMenu === menuName ? t.hoverBg : 'transparent',
                                        border: 'none', color: activeMenu === menuName ? t.textColor : t.textMuted,
                                        fontSize: 12, padding: '3px 10px', borderRadius: 4, cursor: 'pointer',
                                    }}>
                                    {menuName}
                                </button>
                                {activeMenu === menuName && (
                                    <div onClick={(e) => e.stopPropagation()} style={{
                                        position: 'absolute', top: '100%', left: 0, marginTop: 2,
                                        background: t.headerBg, border: `1px solid ${t.headerBorder}`,
                                        borderRadius: 6, padding: 4, minWidth: 200, zIndex: 100,
                                        boxShadow: '0 8px 32px rgba(0,0,0,0.35)',
                                    }}>
                                        {menuDefs[menuName].map((item, i) =>
                                            item.divider ? (
                                                <div key={i} style={{ height: 1, background: t.headerBorder, margin: '4px 8px' }} />
                                            ) : (
                                                <button key={i} disabled={item.disabled}
                                                    onClick={(e) => { e.stopPropagation(); setActiveMenu(null); item.action?.(); }}
                                                    style={{
                                                        display: 'flex', alignItems: 'center', gap: 8, width: '100%',
                                                        padding: '5px 10px', border: 'none', borderRadius: 4, cursor: item.disabled ? 'default' : 'pointer',
                                                        background: 'transparent', color: item.disabled ? t.textMuted : t.textColor,
                                                        fontSize: 12, textAlign: 'left', opacity: item.disabled ? 0.4 : 1,
                                                    }}
                                                    onMouseEnter={(e) => { if (!item.disabled) e.currentTarget.style.background = t.hoverBg; }}
                                                    onMouseLeave={(e) => { e.currentTarget.style.background = 'transparent'; }}>
                                                    {item.icon}
                                                    <span style={{ flex: 1 }}>{item.label}</span>
                                                    {item.shortcut && <span style={{ color: t.textMuted, fontSize: 11 }}>{item.shortcut}</span>}
                                                </button>
                                            )
                                        )}
                                    </div>
                                )}
                            </div>
                        ))}
                    </div>
                </div>
                <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                    <button onClick={handleRun} disabled={isRunning} className={isRunning ? 'pulse-on-run' : ''} style={{ background: t.runBg, border: 'none', color: t.runText, padding: '4px 14px', borderRadius: 6, cursor: 'pointer', display: 'flex', alignItems: 'center', gap: 6, fontSize: 12, fontWeight: 600, opacity: isRunning ? 0.7 : 1 }}>
                        <Play size={12} fill="currentColor" />
                        {isRunning ? 'Çalışıyor...' : 'Çalıştır'}
                    </button>
                </div>
            </div>

            {/* ===== BODY ===== */}
            <div className="ide-body">
                {/* Activity Bar */}
                <div className="ide-activitybar transition-theme" style={{ background: t.activityBg, borderRight: `1px solid ${t.headerBorder}` }}>
                    <button onClick={() => { setActiveView('files'); setSidebarOpen((p) => activeView === 'files' ? !p : true); }} style={{ color: activeView === 'files' && sidebarOpen ? t.activityActive : t.activityIcon }} title="Dosyalar"><Files size={22} /></button>
                    <button onClick={() => { setActiveView('search'); setSidebarOpen((p) => activeView === 'search' ? !p : true); }} style={{ color: activeView === 'search' && sidebarOpen ? t.activityActive : t.activityIcon }} title="Ara"><Search size={22} /></button>
                    <button onClick={() => { setActiveView('git'); setSidebarOpen((p) => activeView === 'git' ? !p : true); }} style={{ color: activeView === 'git' && sidebarOpen ? t.activityActive : t.activityIcon }} title="Kaynak Kontrolü">
                        <div style={{ position: 'relative' }}>
                            <GitGraph size={22} />
                            {files.length > 1 && <div style={{ position: 'absolute', top: -2, right: -2, width: 8, height: 8, background: '#4EC278', borderRadius: '50%' }} />}
                        </div>
                    </button>
                    <button onClick={() => { setActiveView('debug'); setSidebarOpen((p) => activeView === 'debug' ? !p : true); }} style={{ color: activeView === 'debug' && sidebarOpen ? t.activityActive : t.activityIcon }} title="Hata Ayıklama"><Bug size={22} /></button>
                    <div style={{ flex: 1 }} />
                    <button onClick={() => setTerminalOpen((p) => !p)} style={{ color: terminalOpen ? t.activityActive : t.activityIcon, marginBottom: 8 }} title="Terminal"><SquareTerminal size={22} /></button>
                </div>

                {/* Sidebar */}
                <div className={`ide-sidebar transition-theme ${sidebarOpen ? '' : 'collapsed'}`} style={{ background: t.sidebarBg, borderRight: sidebarOpen ? `1px solid ${t.headerBorder}` : 'none' }}>
                    {sidebarOpen && (
                        <>
                            <div className="ide-sidebar-header" style={{ color: t.textMuted, borderBottom: `1px solid ${t.headerBorder}` }}>
                                <span>
                                    {activeView === 'files' ? 'DOSYALAR' :
                                        activeView === 'search' ? 'ARAMA' :
                                            activeView === 'git' ? 'KAYNAK KONTROLÜ' : 'HATA AYIKLAMA'}
                                </span>
                                {activeView === 'files' && (
                                    <div style={{ display: 'flex', gap: 2 }}>
                                        <button onClick={() => createNewFile()} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer', padding: 2, borderRadius: 4 }} title="Yeni Dosya"><Plus size={15} /></button>
                                        <button onClick={() => createFolder()} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer', padding: 2, borderRadius: 4 }} title="Yeni Klasör"><FolderPlus size={15} /></button>
                                        <label style={{ display: 'flex', cursor: 'pointer', padding: 2, alignItems: 'center', justifyContent: 'center' }} title="Dosya Aç">
                                            <FolderOpen size={15} style={{ color: t.textMuted }} />
                                            <input type="file" accept=".tr,.txt,.c,.h,.js,.jsx,.ts,.tsx,.py,.java,.cpp,.hpp,.cs,.go,.rs,.rb,.php,.html,.htm,.css" onChange={handleFileUpload} style={{ display: 'none' }} />
                                        </label>
                                    </div>
                                )}
                            </div>
                            <div className="ide-sidebar-files">
                                {activeView === 'files' && (() => {
                                    const renderTree = (parentId = null, depth = 0) => {
                                        const childFolders = folders.filter(f => f.parentId === parentId).sort((a, b) => a.name.localeCompare(b.name));
                                        const childFiles = files.filter(f => f.parentId === parentId).sort((a, b) => a.name.localeCompare(b.name));
                                        return (
                                            <>
                                                {childFolders.map(folder => {
                                                    const isOpen = expandedFolders[folder.id];
                                                    const isRenaming = renamingFileId === `folder_${folder.id}`;
                                                    return (
                                                        <div key={`folder_${folder.id}`}>
                                                            <div className="ide-file-item"
                                                                onClick={() => toggleFolder(folder.id)}
                                                                onContextMenu={(e) => { e.preventDefault(); setContextMenu({ x: e.clientX, y: e.clientY, folderId: folder.id }); }}
                                                                onDoubleClick={(e) => { e.preventDefault(); e.stopPropagation(); setRenamingFileId(`folder_${folder.id}`); setRenameValue(folder.name); }}
                                                                style={{ color: t.textColor, display: 'flex', alignItems: 'center', gap: 5, cursor: 'pointer', paddingLeft: 8 + depth * 16 }}>
                                                                <ChevronRight size={12} style={{ transform: isOpen ? 'rotate(90deg)' : 'rotate(0deg)', transition: 'transform 0.15s', flexShrink: 0, opacity: 0.6 }} />
                                                                {isOpen ? <FolderOpen size={14} style={{ color: '#e8a838', flexShrink: 0 }} /> : <Folder size={14} style={{ color: '#e8a838', flexShrink: 0 }} />}
                                                                {isRenaming ? (
                                                                    <input ref={renameInputRef} value={renameValue} onChange={(e) => setRenameValue(e.target.value)}
                                                                        onBlur={() => { if (renameValue.trim()) renameFolder(folder.id, renameValue.trim()); setRenamingFileId(null); }}
                                                                        onKeyDown={(e) => { if (e.key === 'Enter') { if (renameValue.trim()) renameFolder(folder.id, renameValue.trim()); setRenamingFileId(null); } if (e.key === 'Escape') setRenamingFileId(null); }}
                                                                        onClick={(e) => e.stopPropagation()}
                                                                        style={{ flex: 1, background: t.editorBg, border: `1px solid ${t.keyword}`, borderRadius: 3, padding: '1px 4px', color: t.textColor, fontSize: 12, outline: 'none', minWidth: 0 }}
                                                                    />
                                                                ) : (
                                                                    <span style={{ overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap', flex: 1, fontWeight: 500 }}>{folder.name}</span>
                                                                )}
                                                            </div>
                                                            {isOpen && renderTree(folder.id, depth + 1)}
                                                        </div>
                                                    );
                                                })}
                                                {childFiles.map((f) => (
                                                    <div key={f.id} className="ide-file-item"
                                                        onClick={() => { if (renamingFileId !== f.id) openFile(f.id); }}
                                                        onContextMenu={(e) => { e.preventDefault(); setContextMenu({ x: e.clientX, y: e.clientY, fileId: f.id }); }}
                                                        onDoubleClick={(e) => { e.preventDefault(); e.stopPropagation(); startRename(f.id); }}
                                                        style={{ color: t.textColor, background: activeTab === f.id ? t.hoverBg : 'transparent', display: 'flex', alignItems: 'center', gap: 6, cursor: 'pointer', paddingLeft: 8 + (depth + (parentId !== null ? 0 : 0)) * 16 + (parentId !== null ? 16 : 0) }}>
                                                        <span style={{ fontSize: 13, flexShrink: 0 }}>{getFileIcon(f.name)}</span>
                                                        {renamingFileId === f.id ? (
                                                            <input ref={renameInputRef} value={renameValue} onChange={(e) => setRenameValue(e.target.value)}
                                                                onBlur={commitRename}
                                                                onKeyDown={(e) => { if (e.key === 'Enter') commitRename(); if (e.key === 'Escape') { setRenamingFileId(null); } }}
                                                                onClick={(e) => e.stopPropagation()}
                                                                style={{ flex: 1, background: t.editorBg, border: `1px solid ${t.keyword}`, borderRadius: 3, padding: '1px 4px', color: t.textColor, fontSize: 12, outline: 'none', minWidth: 0 }}
                                                            />
                                                        ) : (
                                                            <span style={{ overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap', flex: 1 }}>{f.name}</span>
                                                        )}
                                                    </div>
                                                ))}
                                            </>
                                        );
                                    };
                                    return renderTree();
                                })()}

                                {activeView === 'search' && (
                                    <div style={{ padding: '8px 12px' }}>
                                        <input type="text" placeholder="Dosyalarda ara..." value={searchQuery} onChange={(e) => setSearchQuery(e.target.value)}
                                            style={{ width: '100%', background: t.editorBg, border: `1px solid ${t.headerBorder}`, borderRadius: 4, padding: '6px 8px', color: t.textColor, fontSize: 12, outline: 'none', boxSizing: 'border-box' }}
                                        />
                                        {searchQuery.length >= 2 && (
                                            <div style={{ fontSize: 11, color: t.textMuted, margin: '6px 0 4px', fontWeight: 500 }}>
                                                {searchResults.length} sonuç bulundu
                                            </div>
                                        )}
                                        <div style={{ maxHeight: 'calc(100vh - 200px)', overflowY: 'auto' }}>
                                            {searchResults.map((r, i) => (
                                                <button key={i} onClick={() => { openFile(r.fileId); }}
                                                    style={{
                                                        display: 'block', width: '100%', textAlign: 'left', padding: '4px 0',
                                                        background: 'transparent', border: 'none', cursor: 'pointer', borderBottom: `1px solid ${t.headerBorder}`,
                                                    }}>
                                                    <div style={{ fontSize: 11, color: t.keyword, fontWeight: 600 }}>
                                                        {r.fileName} <span style={{ color: t.textMuted, fontWeight: 400 }}>:{r.line}</span>
                                                    </div>
                                                    <div style={{ fontSize: 11, color: t.textColor, whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis', opacity: 0.8 }}>
                                                        {r.text}
                                                    </div>
                                                </button>
                                            ))}
                                        </div>
                                    </div>
                                )}

                                {activeView === 'git' && (
                                    <div style={{ padding: '12px' }}>
                                        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 12 }}>
                                            <span style={{ fontSize: 11, color: t.textMuted, fontWeight: 600 }}>DEĞİŞİKLİKLER</span>
                                            <div style={{ background: t.keyword, color: '#fff', fontSize: 10, padding: '1px 5px', borderRadius: 8 }}>{files.length}</div>
                                        </div>
                                        <div style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
                                            <div style={{ display: 'flex', gap: 8 }}>
                                                <input type="text" placeholder="Mesaj (örn: güncelleme)" style={{ flex: 1, background: t.editorBg, border: `1px solid ${t.headerBorder}`, padding: '4px 8px', borderRadius: 4, color: t.textColor, fontSize: 12, outline: 'none' }} />
                                            </div>
                                            <button style={{ background: t.func, border: 'none', color: '#fff', padding: '6px', borderRadius: 4, fontSize: 12, cursor: 'pointer', fontWeight: 500 }}>
                                                Tümünü Commit Et
                                            </button>
                                        </div>
                                        <div style={{ marginTop: 16, display: 'flex', flexDirection: 'column', gap: 2 }}>
                                            {files.map(f => (
                                                <div key={f.id} style={{ display: 'flex', alignItems: 'center', gap: 6, padding: '4px 0', cursor: 'pointer', color: t.textColor, fontSize: 13 }}>
                                                    <span style={{ fontSize: 12, color: '#e5c07b', fontWeight: 700 }}>M</span>
                                                    {fileIdCounter > 5 && f.id > 1 ? <span style={{ color: '#98c379', fontSize: 12, fontWeight: 700 }}>U</span> : null}
                                                    <span style={{ fontSize: 13 }}>{f.name}</span>
                                                    <span style={{ fontSize: 11, color: t.textMuted, marginLeft: 'auto' }}>src/</span>
                                                </div>
                                            ))}
                                        </div>
                                    </div>
                                )}

                                {activeView === 'debug' && (
                                    <div style={{ padding: '12px' }}>
                                        <div style={{ display: 'flex', gap: 8, marginBottom: 16 }}>
                                            <button style={{ background: t.runBg, border: 'none', color: t.runText, padding: 6, borderRadius: 4, flex: 1, display: 'flex', justifyContent: 'center' }}><PlayCircle size={16} /></button>
                                            <button style={{ background: t.buttonBg, border: 'none', color: t.textColor, padding: 6, borderRadius: 4, flex: 1, display: 'flex', justifyContent: 'center' }}><StepForward size={16} /></button>
                                            <button style={{ background: t.buttonBg, border: 'none', color: t.textColor, padding: 6, borderRadius: 4, flex: 1, display: 'flex', justifyContent: 'center' }}><StepBack size={16} /></button>
                                        </div>

                                        <div style={{ marginBottom: 16 }}>
                                            <div style={{ fontSize: 11, color: t.textMuted, fontWeight: 600, marginBottom: 4 }}>DEĞİŞKENLER</div>
                                            <div style={{ background: t.editorBg, border: `1px solid ${t.headerBorder}`, borderRadius: 4, padding: 8 }}>
                                                <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 12, marginBottom: 4 }}>
                                                    <span style={{ color: '#e5c07b' }}>sayaç:</span>
                                                    <span style={{ color: '#98c379' }}>42</span>
                                                </div>
                                                <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: 12 }}>
                                                    <span style={{ color: '#e5c07b' }}>isim:</span>
                                                    <span style={{ color: '#d19a66' }}>"Mert"</span>
                                                </div>
                                            </div>
                                        </div>

                                        <div>
                                            <div style={{ fontSize: 11, color: t.textMuted, fontWeight: 600, marginBottom: 4 }}>ÇAĞRI YIĞINI</div>
                                            <div style={{ fontSize: 12, color: t.textColor, display: 'flex', flexDirection: 'column', gap: 4 }}>
                                                <div style={{ padding: '2px 4px', background: t.hoverBg, borderRadius: 2 }}>main @ satır 12</div>
                                                <div style={{ padding: '2px 4px', opacity: 0.6 }}>hesapla @ satır 5</div>
                                                <div style={{ padding: '2px 4px', opacity: 0.6 }}>başlat @ satır 1</div>
                                            </div>
                                        </div>
                                    </div>
                                )}
                            </div>
                        </>
                    )}
                </div>

                {/* Main editor + terminal */}
                <div className="ide-main">
                    {/* Tab bar */}
                    <div className="ide-tabbar transition-theme" style={{ background: t.tabBg, borderBottom: `1px solid ${t.headerBorder}` }}>
                        {openTabs.map((id) => {
                            const f = files.find((fi) => fi.id === id);
                            if (!f) return null;
                            const isActive = activeTab === id;
                            return (
                                <button key={id} className={`ide-tab ${isActive ? 'active' : ''}`} onClick={() => setActiveTab(id)}
                                    style={{
                                        background: isActive ? t.tabActiveBg : 'transparent',
                                        color: isActive ? t.textColor : t.textMuted,
                                        borderBottom: isActive ? `2px solid ${t.keyword}` : '2px solid transparent',
                                    }}>
                                    <span style={{ fontSize: 12 }}>{getFileIcon(f.name)}</span>
                                    {f.name}
                                    {isFileModified(id) && <span style={{ color: t.keyword, fontSize: 18, lineHeight: 1, marginLeft: 2 }}>●</span>}
                                    <span className="tab-close" onClick={(e) => closeTab(id, e)} style={{ color: t.textMuted }}><X size={12} /></span>
                                </button>
                            );
                        })}
                        <button onClick={createNewFile} style={{ background: 'transparent', border: 'none', padding: '6px 10px', cursor: 'pointer', color: t.textMuted, display: 'flex', alignItems: 'center' }} title="Yeni Sekme"><Plus size={14} /></button>
                    </div>

                    {/* Breadcrumb */}
                    {activeFile && (() => {
                        const parts = [];
                        let pid = activeFile.parentId;
                        while (pid !== null && pid !== undefined) {
                            const pf = folders.find(f => f.id === pid);
                            if (!pf) break;
                            parts.unshift(pf.name);
                            pid = pf.parentId;
                        }
                        parts.unshift('proje');
                        parts.push(activeFile.name);
                        return (
                            <div style={{ display: 'flex', alignItems: 'center', gap: 4, padding: '3px 14px', background: t.editorBg, borderBottom: `1px solid ${t.headerBorder}`, fontSize: 12, color: t.textMuted, flexShrink: 0 }}>
                                {parts.map((p, i) => (
                                    <span key={i} style={{ display: 'flex', alignItems: 'center', gap: 4 }}>
                                        {i > 0 && <ChevronRight size={10} style={{ opacity: 0.4 }} />}
                                        <span style={{ opacity: i === parts.length - 1 ? 1 : 0.6, color: i === parts.length - 1 ? t.textColor : t.textMuted }}>{p}</span>
                                    </span>
                                ))}
                            </div>
                        );
                    })()}

                    {/* Find & Replace Bar */}
                    {showFindReplace && (
                        <div style={{ display: 'flex', gap: 8, alignItems: 'center', padding: '6px 14px', background: t.headerBg, borderBottom: `1px solid ${t.headerBorder}`, flexShrink: 0, flexWrap: 'wrap' }}>
                            <div style={{ display: 'flex', alignItems: 'center', gap: 4, flex: 1, minWidth: 200 }}>
                                <Search size={13} style={{ color: t.textMuted, flexShrink: 0 }} />
                                <input type="text" placeholder="Bul..." value={findText} onChange={(e) => setFindText(e.target.value)}
                                    autoFocus
                                    onKeyDown={(e) => { if (e.key === 'Enter') findNext(); if (e.key === 'Escape') setShowFindReplace(false); }}
                                    style={{ flex: 1, background: t.editorBg, border: `1px solid ${t.headerBorder}`, borderRadius: 4, padding: '3px 8px', color: t.textColor, fontSize: 12, outline: 'none', minWidth: 80 }} />
                                <span style={{ fontSize: 11, color: t.textMuted, whiteSpace: 'nowrap' }}>
                                    {findMatches.length > 0 ? `${findIndex + 1}/${findMatches.length}` : findText ? '0 sonuç' : ''}
                                </span>
                                <button onClick={findPrev} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer', padding: 2 }} title="Önceki">▲</button>
                                <button onClick={findNext} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer', padding: 2 }} title="Sonraki">▼</button>
                            </div>
                            <div style={{ display: 'flex', alignItems: 'center', gap: 4, flex: 1, minWidth: 200 }}>
                                <Replace size={13} style={{ color: t.textMuted, flexShrink: 0 }} />
                                <input type="text" placeholder="Değiştir..." value={replaceText} onChange={(e) => setReplaceText(e.target.value)}
                                    onKeyDown={(e) => { if (e.key === 'Enter') replaceOne(); if (e.key === 'Escape') setShowFindReplace(false); }}
                                    style={{ flex: 1, background: t.editorBg, border: `1px solid ${t.headerBorder}`, borderRadius: 4, padding: '3px 8px', color: t.textColor, fontSize: 12, outline: 'none', minWidth: 80 }} />
                                <button onClick={replaceOne} style={{ background: t.buttonBg, border: 'none', color: t.textColor, cursor: 'pointer', padding: '2px 8px', borderRadius: 3, fontSize: 11 }}>Değiştir</button>
                                <button onClick={replaceAll} style={{ background: t.buttonBg, border: 'none', color: t.textColor, cursor: 'pointer', padding: '2px 8px', borderRadius: 3, fontSize: 11 }}>Tümü</button>
                            </div>
                            <button onClick={() => setShowFindReplace(false)} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer', padding: 2, flexShrink: 0 }}><X size={14} /></button>
                        </div>
                    )}

                    {/* Editor */}
                    {activeFile ? (
                        <div className="ide-editor-wrap" style={{ background: t.editorBg, display: 'flex' }}>
                            {settings.showLineNumbers && (
                                <div ref={lineNumRef} className="line-numbers editor-scroll transition-theme" style={{ background: t.lineNumBg, color: t.lineNumColor, borderRight: `1px solid ${t.headerBorder}`, fontSize: settings.fontSize }}>
                                    {lines.map((_, idx) => (
                                        <div key={idx} style={{ color: activeLine === idx + 1 ? t.lineNumActive : t.lineNumColor, fontWeight: activeLine === idx + 1 ? 600 : 400, paddingRight: 4 }}>{idx + 1}</div>
                                    ))}
                                </div>
                            )}
                            <div style={{ position: 'relative', flex: 1, overflow: 'hidden' }}>
                                {/* Active Line Highlight */}
                                <div style={{ position: 'absolute', top: (activeLine - 1) * (settings.fontSize * 1.7), left: 0, right: 0, height: settings.fontSize * 1.7, background: t.keyword + '0D', borderLeft: `2px solid ${t.keyword}44`, pointerEvents: 'none', zIndex: 0 }} />

                                {/* Indentation Guides */}
                                {settings.indentGuides && (
                                    <div style={{ position: 'absolute', top: 0, left: 0, right: 0, bottom: 0, pointerEvents: 'none', zIndex: 0 }}>
                                        {lines.map((line, idx) => {
                                            const indent = line.match(/^(\s*)/)[1].length;
                                            const guides = [];
                                            for (let g = 4; g <= indent; g += 4) {
                                                guides.push(
                                                    <div key={`${idx}-${g}`} style={{
                                                        position: 'absolute',
                                                        top: idx * (settings.fontSize * 1.7),
                                                        left: `${g * 0.6}em`,
                                                        width: 1,
                                                        height: settings.fontSize * 1.7,
                                                        background: t.headerBorder,
                                                        opacity: 0.4,
                                                    }} />
                                                );
                                            }
                                            return guides;
                                        })}
                                    </div>
                                )}

                                <pre ref={highlightRef} className="editor-highlight editor-scroll" dangerouslySetInnerHTML={{ __html: highlightedHtml }} aria-hidden="true" style={{ fontSize: settings.fontSize, whiteSpace: settings.wordWrap ? 'pre-wrap' : 'pre', wordBreak: settings.wordWrap ? 'break-all' : 'normal' }} />
                                <textarea ref={textareaRef} value={code} onChange={(e) => updateFileContent(activeTab, e.target.value)} onScroll={handleScroll} onKeyDown={handleKeyDown} onKeyUp={handleKeyUp}
                                    onClick={() => { handleSelectWithHighlight(); setSuggestions([]); }}
                                    spellCheck={false} autoComplete="off" autoCorrect="off" autoCapitalize="off" className="editor-textarea editor-scroll"
                                    style={{ color: 'transparent', caretColor: t.lineNumActive, fontSize: settings.fontSize, whiteSpace: settings.wordWrap ? 'pre-wrap' : 'pre', wordBreak: settings.wordWrap ? 'break-all' : 'normal', overflowWrap: settings.wordWrap ? 'break-word' : 'normal' }} />

                                {/* IntelliSense Popup */}
                                {suggestions.length > 0 && (
                                    <div style={{
                                        position: 'absolute', top: suggestionPos.top + 2, left: suggestionPos.left,
                                        background: t.editorBg, border: `1px solid ${t.headerBorder}`,
                                        borderRadius: 4, padding: 0, minWidth: 150, zIndex: 100,
                                        boxShadow: '0 4px 12px rgba(0,0,0,0.3)', maxHeight: 200, overflowY: 'auto'
                                    }}>
                                        {suggestions.map((s, i) => (
                                            <div key={i} onClick={() => applySuggestion(s)}
                                                style={{
                                                    padding: '4px 8px', fontSize: 12, cursor: 'pointer',
                                                    background: i === suggestionIndex ? t.keyword + '44' : 'transparent',
                                                    color: t.textColor, display: 'flex', justifyContent: 'space-between'
                                                }}>
                                                <span style={{ fontWeight: 600 }}>{s.text}</span>
                                                <span style={{ opacity: 0.6, fontSize: 10 }}>{s.type === 'keyword' ? '🗝️' : 'ƒ'}</span>
                                            </div>
                                        ))}
                                    </div>
                                )}
                            </div>

                            {/* Minimap (Simple visual) */}
                            {settings.showMinimap && (
                                <div
                                    style={{ width: 60, borderLeft: `1px solid ${t.headerBorder}`, background: t.bg, opacity: 0.8, userSelect: 'none', cursor: 'pointer', position: 'relative' }}
                                    onClick={(e) => {
                                        const rect = e.currentTarget.getBoundingClientRect();
                                        const y = e.clientY - rect.top;
                                        const percentage = y / rect.height;
                                        if (textareaRef.current) {
                                            textareaRef.current.scrollTop = percentage * (textareaRef.current.scrollHeight - textareaRef.current.clientHeight);
                                        }
                                    }}
                                >
                                    <pre style={{ fontSize: 2, margin: 0, paddingTop: 10, color: t.textColor, lineHeight: 1.2, pointerEvents: 'none' }}>
                                        {code}
                                    </pre>
                                    {/* Minimap Viewport Indicator */}
                                    {textareaRef.current && (
                                        <div style={{
                                            position: 'absolute',
                                            top: (textareaRef.current.scrollTop / textareaRef.current.scrollHeight) * 100 + '%',
                                            height: (textareaRef.current.clientHeight / textareaRef.current.scrollHeight) * 100 + '%',
                                            width: '100%',
                                            background: t.textMuted,
                                            opacity: 0.2,
                                            pointerEvents: 'none'
                                        }} />
                                    )}
                                </div>
                            )}
                        </div>
                    ) : (
                        <div className="welcome-screen editor-scroll" style={{
                            background: 'linear-gradient(135deg, #0f172a 0%, #1e1b4b 100%)',
                            color: '#e2e8f0',
                            height: '100%',
                            overflowY: 'auto',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            justifyContent: 'center',
                            position: 'relative'
                        }}>
                            {/* Animated Background Elements */}
                            <div style={{ position: 'absolute', top: '10%', left: '10%', width: 300, height: 300, background: '#3b82f6', filter: 'blur(100px)', opacity: 0.2, borderRadius: '50%' }} />
                            <div style={{ position: 'absolute', bottom: '10%', right: '10%', width: 300, height: 300, background: '#8b5cf6', filter: 'blur(100px)', opacity: 0.2, borderRadius: '50%' }} />

                            <div style={{ width: '100%', maxWidth: 900, padding: 40, animation: 'fadeIn 0.5s ease-out', zIndex: 1 }}>
                                {/* Hero Section */}
                                <div style={{ marginBottom: 48, textAlign: 'center' }}>
                                    <div style={{
                                        display: 'inline-flex', alignItems: 'center', justifyContent: 'center',
                                        width: 96, height: 96, borderRadius: 28,
                                        background: 'rgba(255, 255, 255, 0.05)',
                                        backdropFilter: 'blur(10px)',
                                        marginBottom: 24,
                                        boxShadow: '0 8px 32px rgba(0, 0, 0, 0.2)',
                                        border: '1px solid rgba(255, 255, 255, 0.1)'
                                    }}>
                                        <Code2 size={48} style={{ color: '#60a5fa', filter: 'drop-shadow(0 0 10px rgba(96, 165, 250, 0.5))' }} />
                                    </div>
                                    <h1 style={{
                                        fontSize: 48, fontWeight: 800, margin: '0 0 16px', letterSpacing: '-0.02em',
                                        background: 'linear-gradient(to right, #ffffff, #94a3b8)',
                                        WebkitBackgroundClip: 'text', WebkitTextFillColor: 'transparent',
                                        dropShadow: '0 4px 12px rgba(0,0,0,0.5)'
                                    }}>
                                        Mert Studio Code
                                    </h1>
                                    <p style={{ margin: 0, color: '#94a3b8', fontSize: 18, fontWeight: 400 }}>Modern, Hızlı ve Türkçe Kodlama Deneyimi</p>
                                    <div style={{ marginTop: 8, display: 'inline-block', padding: '4px 12px', borderRadius: 20, background: 'rgba(59, 130, 246, 0.1)', border: '1px solid rgba(59, 130, 246, 0.2)', fontSize: 12, color: '#60a5fa' }}>Sürüm 3.5.0</div>
                                </div>

                                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: 32 }}>
                                    {/* Start Section */}
                                    <div style={{
                                        background: 'rgba(30, 41, 59, 0.4)',
                                        backdropFilter: 'blur(12px)',
                                        borderRadius: 24,
                                        border: '1px solid rgba(255, 255, 255, 0.05)',
                                        padding: 32,
                                        display: 'flex',
                                        flexDirection: 'column',
                                        gap: 16,
                                        boxShadow: '0 20px 40px rgba(0,0,0,0.2)'
                                    }}>
                                        <h3 style={{ fontSize: 20, fontWeight: 600, marginBottom: 8, display: 'flex', alignItems: 'center', gap: 12, color: '#f8fafc' }}>
                                            <Zap size={22} style={{ color: '#facc15' }} /> Başlangıç
                                        </h3>
                                        {[
                                            { icon: <FilePlus size={20} />, label: "Yeni Dosya", desc: "Boş bir dosya oluştur", action: () => createNewFile() },
                                            { icon: <FolderPlus size={20} />, label: "Yeni Klasör", desc: "Çalışma alanına klasör ekle", action: () => createFolder() },
                                            { icon: <GitGraph size={20} />, label: "Git Klonla", desc: "Bir depoyu kopyala", action: () => { setActiveView('git'); setSidebarOpen(true); } }
                                        ].map((item, i) => (
                                            <button key={i} onClick={item.action}
                                                style={{
                                                    display: 'flex', alignItems: 'center', gap: 16, padding: '16px', width: '100%',
                                                    background: 'rgba(255, 255, 255, 0.03)', border: '1px solid rgba(255, 255, 255, 0.05)', borderRadius: 16,
                                                    color: '#e2e8f0', cursor: 'pointer', textAlign: 'left', transition: 'all 0.2s',
                                                }}
                                                onMouseEnter={(e) => {
                                                    e.currentTarget.style.background = 'rgba(255, 255, 255, 0.08)';
                                                    e.currentTarget.style.borderColor = '#60a5fa';
                                                    e.currentTarget.style.transform = 'translateY(-2px)';
                                                    e.currentTarget.style.boxShadow = '0 8px 16px rgba(0,0,0,0.2)';
                                                }}
                                                onMouseLeave={(e) => {
                                                    e.currentTarget.style.background = 'rgba(255, 255, 255, 0.03)';
                                                    e.currentTarget.style.borderColor = 'rgba(255, 255, 255, 0.05)';
                                                    e.currentTarget.style.transform = 'translateY(0)';
                                                    e.currentTarget.style.boxShadow = 'none';
                                                }}
                                            >
                                                <div style={{ padding: 10, borderRadius: 12, background: 'rgba(59, 130, 246, 0.2)', color: '#60a5fa' }}>{item.icon}</div>
                                                <div>
                                                    <div style={{ fontWeight: 600, fontSize: 15 }}>{item.label}</div>
                                                    <div style={{ fontSize: 13, color: '#94a3b8', marginTop: 2 }}>{item.desc}</div>
                                                </div>
                                            </button>
                                        ))}
                                        <label style={{ display: 'block', cursor: 'pointer' }}>
                                            <div style={{
                                                display: 'flex', alignItems: 'center', gap: 16, padding: '16px', width: '100%',
                                                background: 'rgba(255, 255, 255, 0.03)', border: '1px solid rgba(255, 255, 255, 0.05)', borderRadius: 16,
                                                color: '#e2e8f0', textAlign: 'left', transition: 'all 0.2s',
                                            }}
                                                onMouseEnter={(e) => {
                                                    e.currentTarget.style.background = 'rgba(255, 255, 255, 0.08)';
                                                    e.currentTarget.style.borderColor = '#60a5fa';
                                                    e.currentTarget.style.transform = 'translateY(-2px)';
                                                }}
                                                onMouseLeave={(e) => {
                                                    e.currentTarget.style.background = 'rgba(255, 255, 255, 0.03)';
                                                    e.currentTarget.style.borderColor = 'rgba(255, 255, 255, 0.05)';
                                                    e.currentTarget.style.transform = 'translateY(0)';
                                                }}
                                            >
                                                <div style={{ padding: 10, borderRadius: 12, background: 'rgba(59, 130, 246, 0.2)', color: '#60a5fa' }}>
                                                    <FolderOpen size={20} />
                                                </div>
                                                <div>
                                                    <div style={{ fontWeight: 600, fontSize: 15 }}>Dosya Aç</div>
                                                    <div style={{ fontSize: 13, color: '#94a3b8', marginTop: 2 }}>Bilgisayardan dosya seç</div>
                                                </div>
                                            </div>
                                            <input type="file" accept=".tr,.txt,.c,.h,.js,.jsx,.ts,.tsx,.py,.java,.cpp,.hpp,.cs,.go,.rs,.rb,.php,.html,.htm,.css" onChange={handleFileUpload} style={{ display: 'none' }} />
                                        </label>
                                    </div>

                                    {/* Recent & Help Section */}
                                    <div style={{ display: 'flex', flexDirection: 'column', gap: 24 }}>
                                        <div style={{
                                            background: 'rgba(30, 41, 59, 0.4)',
                                            backdropFilter: 'blur(12px)',
                                            borderRadius: 24,
                                            border: '1px solid rgba(255, 255, 255, 0.05)',
                                            padding: 32,
                                            flex: 1,
                                            display: 'flex',
                                            flexDirection: 'column',
                                            boxShadow: '0 20px 40px rgba(0,0,0,0.2)'
                                        }}>
                                            <h3 style={{ fontSize: 20, fontWeight: 600, marginBottom: 16, display: 'flex', alignItems: 'center', gap: 12, color: '#f8fafc' }}>
                                                <Clock size={22} style={{ color: '#a78bfa' }} /> Son Dosyalar
                                            </h3>
                                            {files.length === 0 ? (
                                                <div style={{ flex: 1, display: 'flex', alignItems: 'center', justifyContent: 'center', padding: 20, textAlign: 'center', color: '#64748b', fontStyle: 'italic', border: '2px dashed rgba(255,255,255,0.1)', borderRadius: 16, background: 'rgba(255,255,255,0.01)' }}>
                                                    Henüz açık dosya yok
                                                </div>
                                            ) : (
                                                <div style={{ display: 'flex', flexDirection: 'column', gap: 8 }}>
                                                    {files.slice(0, 5).map(f => (
                                                        <button key={f.id} onClick={() => openFile(f.id)}
                                                            style={{
                                                                textAlign: 'left', padding: '12px 16px', borderRadius: 12, border: 'none',
                                                                background: 'transparent', color: '#cbd5e1', cursor: 'pointer',
                                                                display: 'flex', alignItems: 'center', gap: 12, fontSize: 14,
                                                                transition: 'background 0.2s',
                                                                border: '1px solid transparent'
                                                            }}
                                                            onMouseEnter={(e) => { e.currentTarget.style.background = 'rgba(255, 255, 255, 0.05)'; e.currentTarget.style.border = '1px solid rgba(255,255,255,0.05)'; }}
                                                            onMouseLeave={(e) => { e.currentTarget.style.background = 'transparent'; e.currentTarget.style.border = '1px solid transparent'; }}
                                                        >
                                                            <FileCode2 size={18} style={{ color: '#a78bfa', minWidth: 18 }} />
                                                            <div style={{ display: 'flex', flexDirection: 'column', overflow: 'hidden' }}>
                                                                <span style={{ fontWeight: 500, overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap', color: '#f1f5f9' }}>{f.name}</span>
                                                                <span style={{ fontSize: 12, color: '#64748b', whiteSpace: 'nowrap', overflow: 'hidden', textOverflow: 'ellipsis' }}>
                                                                    {f.parentId ? folders.find(fo => fo.id === f.parentId)?.name : './'}
                                                                </span>
                                                            </div>
                                                        </button>
                                                    ))}
                                                </div>
                                            )}
                                        </div>

                                        <div style={{ display: 'flex', gap: 12, justifyContent: 'center', opacity: 0.7 }}>
                                            {[
                                                { i: <Command size={14} />, l: "F1", t: "Komutlar" },
                                                { i: <Search size={14} />, l: "Ctrl+F", t: "Ara" }
                                            ].map((h, k) => (
                                                <div key={k} style={{
                                                    display: 'flex', alignItems: 'center', gap: 8, fontSize: 12, color: '#94a3b8',
                                                    background: 'rgba(15, 23, 42, 0.6)', padding: '6px 12px', borderRadius: 20,
                                                    border: '1px solid rgba(255,255,255,0.05)'
                                                }}>
                                                    {h.i} <span>{h.t}: </span> <strong style={{ color: '#e2e8f0' }}>{h.l}</strong>
                                                </div>
                                            ))}
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    )}

                    {/* Terminal */}
                    {terminalOpen && (
                        <div className="ide-terminal-wrap transition-theme" style={{ height: terminalHeight, borderTop: `1px solid ${t.headerBorder}` }}>
                            <div className="ide-terminal-resize"
                                style={{ background: t.headerBorder }}
                                onMouseDown={() => { resizeRef.current = true; document.body.style.cursor = 'ns-resize'; }}
                                onMouseEnter={(e) => { e.currentTarget.style.background = t.keyword; e.currentTarget.style.height = '4px'; }}
                                onMouseLeave={(e) => { if (!resizeRef.current) { e.currentTarget.style.background = t.headerBorder; e.currentTarget.style.height = '2px'; } }}
                            />
                            <div className="ide-terminal-header transition-theme" style={{ background: t.consoleHeader, color: t.textMuted }}>
                                <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                                    <Terminal size={13} style={{ color: t.consoleText }} />
                                    <span>TERMİNAL</span>
                                </div>
                                <div style={{ display: 'flex', gap: 4 }}>
                                    {consoleOutput.length > 0 && (
                                        <button onClick={() => setConsoleOutput([])} style={{ background: 'transparent', border: 'none', cursor: 'pointer', color: t.textMuted, padding: 2 }} title="Temizle"><Trash2 size={13} /></button>
                                    )}
                                    <button onClick={() => setTerminalOpen(false)} style={{ background: 'transparent', border: 'none', cursor: 'pointer', color: t.textMuted, padding: 2 }} title="Kapat"><PanelBottomClose size={13} /></button>
                                </div>
                            </div>
                            <div className="ide-terminal-body editor-scroll transition-theme" style={{ background: t.consoleBg, color: t.textColor, paddingBottom: 10 }}>
                                {consoleOutput.length === 0 && (
                                    <div style={{ opacity: 0.4, marginBottom: 10 }} className="console-cursor">Terminal hazır. 'yardim' yazarak komutları görebilirsiniz.</div>
                                )}
                                {consoleOutput.map((line, idx) => (
                                    <div key={idx} className="fade-in" style={{
                                        color: line.type === 'info' ? t.keyword
                                            : line.type === 'success' ? t.consoleText
                                                : line.type === 'error' ? '#f38ba8'
                                                    : line.type === 'warning' ? '#fab387'
                                                        : line.type === 'command' ? t.textMuted
                                                            : t.textColor,
                                        fontWeight: line.type === 'info' || line.type === 'success' ? 500 : 400,
                                        fontStyle: line.type === 'info' || line.type === 'command' ? 'italic' : 'normal',
                                        paddingLeft: line.type === 'command' ? 0 : 4,
                                    }}>
                                        {line.type === 'output' ? `  ${line.text}` : line.text}
                                    </div>
                                ))}
                                <div style={{ display: 'flex', alignItems: 'center', marginTop: 4 }}>
                                    <span style={{ color: t.func, marginRight: 6 }}>➜</span>
                                    <span style={{ color: t.keyword, marginRight: 6 }}>~</span>
                                    <input
                                        type="text"
                                        value={termInput}
                                        onChange={(e) => setTermInput(e.target.value)}
                                        onKeyDown={handleTerminalCmd}
                                        placeholder="Komut yazın..."
                                        style={{
                                            background: 'transparent', border: 'none', outline: 'none',
                                            color: t.textColor, fontFamily: 'monospace', flex: 1, fontSize: 13
                                        }}
                                        spellCheck={false}
                                        autoComplete="off"
                                    />
                                </div>
                            </div>
                        </div>
                    )}
                </div>
            </div>

            {/* ===== STATUS BAR ===== */}
            <div className="ide-statusbar transition-theme" style={{ background: t.statusBg, color: t.statusText }}>
                <div style={{ display: 'flex', gap: 16 }}>
                    <span>Satır {activeLine}, Sütun {activeCol}</span>
                    <span>{lines.length} satır</span>
                </div>
                <div style={{ display: 'flex', gap: 16, alignItems: 'center' }}>
                    <span>UTF-8</span>
                    {/* Language picker */}
                    <div style={{ position: 'relative' }}>
                        <button onClick={(e) => { e.stopPropagation(); setShowLangPicker((p) => !p); }}
                            style={{ background: 'transparent', border: 'none', color: t.statusText, cursor: 'pointer', display: 'flex', alignItems: 'center', gap: 4, fontSize: 11, padding: '1px 4px', borderRadius: 3 }}
                            onMouseEnter={(e) => e.currentTarget.style.background = 'rgba(255,255,255,0.15)'}
                            onMouseLeave={(e) => e.currentTarget.style.background = 'transparent'}>
                            <Languages size={12} />
                            {LANGUAGES[language]?.name || 'Türkçe'}
                            <ChevronDown size={10} />
                        </button>
                        {showLangPicker && (
                            <div onClick={(e) => e.stopPropagation()} style={{
                                position: 'absolute', bottom: '100%', right: 0, marginBottom: 4,
                                background: t.headerBg, border: `1px solid ${t.headerBorder}`,
                                borderRadius: 6, padding: 4, minWidth: 200, maxHeight: '50vh', overflowY: 'auto', zIndex: 100,
                                boxShadow: '0 -4px 24px rgba(0,0,0,0.35)',
                            }}>
                                {Object.entries(LANGUAGES).map(([id, lang]) => (
                                    <button key={id} onClick={() => { setLanguage(id); setShowLangPicker(false); }}
                                        style={{
                                            display: 'flex', alignItems: 'center', gap: 8, width: '100%',
                                            padding: '5px 10px', border: 'none', borderRadius: 4, cursor: 'pointer',
                                            background: language === id ? t.hoverBg : 'transparent',
                                            color: t.textColor, fontSize: 12, textAlign: 'left',
                                        }}
                                        onMouseEnter={(e) => e.currentTarget.style.background = t.hoverBg}
                                        onMouseLeave={(e) => { if (language !== id) e.currentTarget.style.background = 'transparent'; }}>
                                        <Languages size={13} style={{ color: t.keyword }} />
                                        <div>
                                            <div style={{ fontWeight: language === id ? 600 : 400 }}>{lang.name}</div>
                                            <div style={{ fontSize: 10, color: t.textMuted }}>{lang.extensions.join(', ')}</div>
                                        </div>
                                    </button>
                                ))}
                            </div>
                        )}
                    </div>
                    <span>Mert Studio Code</span>
                </div>
            </div>

            {/* Context Menu */}
            {contextMenu && (
                <div className="context-menu" style={{ left: contextMenu.x, top: contextMenu.y, background: t.headerBg, border: `1px solid ${t.headerBorder}` }}>
                    {contextMenu.folderId ? (
                        <>
                            <button onClick={() => { createNewFile(contextMenu.folderId); setContextMenu(null); }} style={{ color: t.textColor }}>
                                <FilePlus size={13} /> Yeni Dosya
                            </button>
                            <button onClick={() => { createFolder(contextMenu.folderId); setContextMenu(null); }} style={{ color: t.textColor }}>
                                <FolderPlus size={13} /> Yeni Alt Klasör
                            </button>
                            <button onClick={() => { setRenamingFileId(`folder_${contextMenu.folderId}`); const fol = folders.find(f => f.id === contextMenu.folderId); if (fol) setRenameValue(fol.name); setContextMenu(null); }} style={{ color: t.textColor }}>
                                <Pencil size={13} /> Yeniden Adlandır
                            </button>
                            <button onClick={() => { deleteFolder(contextMenu.folderId); setContextMenu(null); }} style={{ color: '#f38ba8' }}>
                                <Trash2 size={13} /> Klasörü Sil
                            </button>
                        </>
                    ) : (
                        <>
                            <button onClick={() => { startRename(contextMenu.fileId); setContextMenu(null); }} style={{ color: t.textColor }}>
                                <Pencil size={13} /> Yeniden Adlandır
                            </button>
                            <button onClick={() => deleteFile(contextMenu.fileId)} style={{ color: '#f38ba8' }}>
                                <Trash2 size={13} /> Dosyayı Sil
                            </button>
                        </>
                    )}
                </div>
            )}

            {/* Theme Selector Modal */}
            {showThemeSelector && (
                <ThemeSelector currentTheme={theme} onSelect={(k) => { setTheme(k); setShowThemeSelector(false); }} onClose={() => setShowThemeSelector(false)} />
            )}

            {/* Settings Modal */}
            {showSettings && (
                <SettingsModal theme={theme} onClose={() => setShowSettings(false)} settings={settings} onUpdateSettings={updateSettings} />
            )}

            {/* Shortcuts Modal */}
            {showShortcuts && (
                <div style={{ position: 'fixed', inset: 0, zIndex: 200, display: 'flex', alignItems: 'center', justifyContent: 'center', background: 'rgba(0,0,0,0.6)', backdropFilter: 'blur(8px)' }} onClick={() => setShowShortcuts(false)}>
                    <div onClick={(e) => e.stopPropagation()} style={{ background: t.bg, borderRadius: 12, padding: 24, minWidth: 360, boxShadow: '0 16px 48px rgba(0,0,0,0.4)', border: `1px solid ${t.headerBorder}` }}>
                        <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: 16 }}>
                            <h3 style={{ color: t.textColor, margin: 0, fontSize: 16 }}>Klavye Kısayolları</h3>
                            <button onClick={() => setShowShortcuts(false)} style={{ background: 'transparent', border: 'none', color: t.textMuted, cursor: 'pointer' }}><X size={16} /></button>
                        </div>
                        {[
                            ['Tab', '4 boşluk girintileme'],
                            ['Enter', 'Otomatik girintileme'],
                            ['Ctrl+Space', 'Kod tamamlamayı aç'],
                            ['Ctrl+Z', 'Geri al'],
                            ['Ctrl+Y', 'İleri al'],
                            ['Ctrl+D', 'Satır kopyala'],
                            ['Ctrl+Shift+K', 'Satır sil'],
                            ['Alt+↑/↓', 'Satır taşı'],
                            ['Ctrl+F / Ctrl+H', 'Bul & Değiştir'],
                            ['Ctrl+Shift+P', 'Komut Paleti'],
                            ['Ctrl+A', 'Tümünü seç'],
                            ['Çift Tık', 'Dosya yeniden adlandır'],
                        ].map(([key, desc]) => (
                            <div key={key} style={{ display: 'flex', justifyContent: 'space-between', padding: '6px 0', borderBottom: `1px solid ${t.headerBorder}` }}>
                                <span style={{ color: t.textMuted, fontSize: 13 }}>{desc}</span>
                                <kbd style={{ background: t.buttonBg, color: t.textColor, padding: '2px 8px', borderRadius: 4, fontSize: 12, fontFamily: 'var(--font-code)' }}>{key}</kbd>
                            </div>
                        ))}
                    </div>
                </div>
            )}

            {/* About Modal */}
            {showAbout && (
                <div style={{ position: 'fixed', inset: 0, zIndex: 200, display: 'flex', alignItems: 'center', justifyContent: 'center', background: 'rgba(0,0,0,0.6)', backdropFilter: 'blur(8px)' }} onClick={() => setShowAbout(false)}>
                    <div onClick={(e) => e.stopPropagation()} style={{ background: t.bg, borderRadius: 12, padding: 24, minWidth: 320, textAlign: 'center', boxShadow: '0 16px 48px rgba(0,0,0,0.4)', border: `1px solid ${t.headerBorder}` }}>
                        <Code2 size={40} style={{ color: t.keyword, marginBottom: 12 }} />
                        <h3 style={{ color: t.textColor, margin: '0 0 8px' }}>Mert Studio Code</h3>
                        <p style={{ color: t.textMuted, fontSize: 13, margin: '0 0 4px' }}>Çoklu Dil Destekli Kod Editörü</p>
                        <p style={{ color: t.textMuted, fontSize: 12, margin: '0 0 4px' }}>V3.5 — The Ultimate Edition</p>
                        <p style={{ color: t.textMuted, fontSize: 12 }}>Sürüm 3.5.0</p>
                        <button onClick={() => setShowAbout(false)} style={{ marginTop: 16, background: t.buttonBg, border: 'none', color: t.textColor, padding: '6px 20px', borderRadius: 6, cursor: 'pointer', fontSize: 12 }}>Tamam</button>
                    </div>
                </div>
            )}

            {/* Command Palette */}
            {showCommandPalette && (
                <div style={{ position: 'fixed', inset: 0, zIndex: 300, display: 'flex', justifyContent: 'center', paddingTop: 80, background: 'rgba(0,0,0,0.5)', backdropFilter: 'blur(4px)' }} onClick={() => setShowCommandPalette(false)}>
                    <div onClick={(e) => e.stopPropagation()} style={{ background: t.bg, borderRadius: 10, width: '100%', maxWidth: 480, height: 'fit-content', maxHeight: '60vh', overflow: 'hidden', boxShadow: '0 16px 64px rgba(0,0,0,0.5)', border: `1px solid ${t.headerBorder}` }}>
                        <div style={{ display: 'flex', alignItems: 'center', gap: 8, padding: '10px 14px', borderBottom: `1px solid ${t.headerBorder}` }}>
                            <Command size={14} style={{ color: t.keyword }} />
                            <input type="text" placeholder="Komut ara..." value={cmdSearch} onChange={(e) => setCmdSearch(e.target.value)} autoFocus
                                onKeyDown={(e) => { if (e.key === 'Escape') setShowCommandPalette(false); if (e.key === 'Enter' && filteredCommands.length > 0) filteredCommands[0].action(); }}
                                style={{ flex: 1, background: 'transparent', border: 'none', outline: 'none', color: t.textColor, fontSize: 14 }} />
                        </div>
                        <div style={{ overflowY: 'auto', maxHeight: 'calc(60vh - 50px)' }}>
                            {filteredCommands.map((cmd, i) => (
                                <button key={i} onClick={cmd.action}
                                    style={{ display: 'flex', alignItems: 'center', gap: 10, width: '100%', padding: '8px 14px', border: 'none', background: 'transparent', color: t.textColor, cursor: 'pointer', fontSize: 13, textAlign: 'left' }}
                                    onMouseEnter={(e) => e.currentTarget.style.background = t.hoverBg}
                                    onMouseLeave={(e) => e.currentTarget.style.background = 'transparent'}>
                                    <span style={{ fontSize: 14, width: 20, textAlign: 'center' }}>{cmd.icon}</span>
                                    <span style={{ flex: 1 }}>{cmd.label}</span>
                                    {cmd.shortcut && <span style={{ color: t.textMuted, fontSize: 11 }}>{cmd.shortcut}</span>}
                                </button>
                            ))}
                            {filteredCommands.length === 0 && (
                                <div style={{ padding: '16px 14px', color: t.textMuted, fontSize: 13, textAlign: 'center' }}>Komut bulunamadı</div>
                            )}
                        </div>
                    </div>
                </div>
            )}
        </div>
    );
}
