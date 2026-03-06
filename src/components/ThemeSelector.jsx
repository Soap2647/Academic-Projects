import { useMemo } from 'react';
import { X, Check, Palette } from 'lucide-react';
import { themes, THEME_META, PREVIEW_CODE, highlightLine } from './editorUtils';

export default function ThemeSelector({ currentTheme, onSelect, onClose }) {
    const themeKeys = Object.keys(themes);
    const ct = themes[currentTheme] || themes.dark;

    const previews = useMemo(() => {
        const map = {};
        for (const key of themeKeys) {
            map[key] = PREVIEW_CODE.split('\n')
                .map((l) => highlightLine(l, key, 'turkce_c'))
                .join('\n');
        }
        return map;
    }, []);

    return (
        <div style={{
            position: 'fixed', inset: 0, zIndex: 200, display: 'flex', alignItems: 'center', justifyContent: 'center',
            background: 'rgba(0,0,0,0.6)', backdropFilter: 'blur(12px)', WebkitBackdropFilter: 'blur(12px)',
            animation: 'fadeIn 0.2s ease-out',
        }} onClick={onClose}>

            <div onClick={(e) => e.stopPropagation()} style={{
                background: ct.bg, borderRadius: 16, width: '92vw', maxWidth: 960, maxHeight: '90vh',
                overflow: 'hidden', display: 'flex', flexDirection: 'column',
                boxShadow: '0 24px 80px rgba(0,0,0,0.5), 0 0 0 1px rgba(255,255,255,0.08)',
                animation: 'modalSlideIn 0.3s ease-out',
            }}>
                {/* Header */}
                <div style={{
                    display: 'flex', alignItems: 'center', justifyContent: 'space-between',
                    padding: '16px 20px 12px', borderBottom: `1px solid ${ct.headerBorder}`, flexShrink: 0,
                }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
                        <div style={{
                            width: 32, height: 32, borderRadius: 8, display: 'flex', alignItems: 'center', justifyContent: 'center',
                            background: `linear-gradient(135deg, ${ct.keyword}, ${ct.func})`,
                        }}>
                            <Palette size={16} style={{ color: '#fff' }} />
                        </div>
                        <div>
                            <div style={{ color: ct.textColor, fontWeight: 700, fontSize: 15 }}>Tema Seçici</div>
                            <div style={{ color: ct.textMuted, fontSize: 11 }}>Editörünüzü kişiselleştirin</div>
                        </div>
                    </div>
                    <button onClick={onClose} style={{
                        background: ct.buttonBg, border: 'none', borderRadius: 8, width: 30, height: 30,
                        display: 'flex', alignItems: 'center', justifyContent: 'center', cursor: 'pointer',
                        color: ct.textMuted, transition: 'all 0.15s',
                    }}><X size={14} /></button>
                </div>

                {/* Theme grid — scrollable */}
                <div style={{
                    padding: '16px 20px 20px', overflowY: 'auto', display: 'grid',
                    gridTemplateColumns: 'repeat(3, 1fr)', gap: 12, flex: 1,
                }}>
                    {themeKeys.map((key) => {
                        const th = themes[key];
                        const meta = THEME_META[key];
                        const isActive = key === currentTheme;
                        return (
                            <button key={key} onClick={() => onSelect(key)}
                                style={{
                                    border: isActive ? `2px solid ${th.keyword}` : `1px solid ${ct.headerBorder}`,
                                    borderRadius: 10, overflow: 'hidden', cursor: 'pointer',
                                    background: th.bg, textAlign: 'left', padding: 0,
                                    transition: 'all 0.2s ease', position: 'relative',
                                    transform: isActive ? 'scale(1.02)' : 'scale(1)',
                                    boxShadow: isActive ? `0 0 16px ${th.keyword}33, 0 4px 12px rgba(0,0,0,0.3)` : '0 2px 6px rgba(0,0,0,0.2)',
                                }}
                                onMouseEnter={(e) => {
                                    if (!isActive) e.currentTarget.style.transform = 'scale(1.02)';
                                    e.currentTarget.style.boxShadow = `0 0 16px ${th.keyword}33, 0 4px 12px rgba(0,0,0,0.3)`;
                                }}
                                onMouseLeave={(e) => {
                                    if (!isActive) e.currentTarget.style.transform = 'scale(1)';
                                    if (!isActive) e.currentTarget.style.boxShadow = '0 2px 6px rgba(0,0,0,0.2)';
                                }}
                            >
                                {isActive && (
                                    <div style={{
                                        position: 'absolute', top: 6, right: 6, width: 20, height: 20,
                                        borderRadius: '50%', background: th.keyword,
                                        display: 'flex', alignItems: 'center', justifyContent: 'center', zIndex: 2,
                                    }}>
                                        <Check size={12} style={{ color: '#fff' }} />
                                    </div>
                                )}

                                {/* Code preview — compact */}
                                <div style={{
                                    padding: '8px 10px', background: th.editorBg, borderBottom: `1px solid ${th.headerBorder}`,
                                    fontFamily: "'JetBrains Mono', monospace", fontSize: 9, lineHeight: 1.5,
                                    overflow: 'hidden', height: 85, position: 'relative',
                                }}>
                                    <div style={{ position: 'absolute', left: 10, top: 8, color: th.lineNumColor, userSelect: 'none' }}>
                                        {PREVIEW_CODE.split('\n').map((_, i) => (
                                            <div key={i} style={{ textAlign: 'right', width: 14 }}>{i + 1}</div>
                                        ))}
                                    </div>
                                    <pre style={{ margin: 0, marginLeft: 24, overflow: 'hidden', whiteSpace: 'pre', color: th.textColor }}
                                        dangerouslySetInnerHTML={{ __html: previews[key] }} />
                                    <div style={{
                                        position: 'absolute', bottom: 0, left: 0, right: 0, height: 24,
                                        background: `linear-gradient(transparent, ${th.editorBg})`,
                                    }} />
                                </div>

                                {/* Info + swatches */}
                                <div style={{ padding: '8px 10px 6px' }}>
                                    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: 2 }}>
                                        <span style={{ color: th.textColor, fontWeight: 600, fontSize: 12 }}>{meta.name}</span>
                                        <span style={{ fontSize: 9, color: th.textMuted, background: th.headerBg, padding: '1px 6px', borderRadius: 4, fontWeight: 500 }}>{meta.group}</span>
                                    </div>
                                    <div style={{ color: th.textMuted, fontSize: 10, marginBottom: 6 }}>{meta.desc}</div>
                                    <div style={{ display: 'flex', gap: 3 }}>
                                        {[th.keyword, th.func, th.string, th.number, th.comment, th.preprocessor].map((c, i) => (
                                            <div key={i} style={{ width: 12, height: 12, borderRadius: '50%', background: c, boxShadow: `0 0 3px ${c}66` }} />
                                        ))}
                                    </div>
                                </div>

                                {/* Status bar */}
                                <div style={{
                                    height: 16, background: th.statusBg, display: 'flex', alignItems: 'center',
                                    padding: '0 8px', borderRadius: '0 0 8px 8px',
                                }}>
                                    <span style={{ fontSize: 8, color: th.statusText, opacity: 0.8 }}>Mert Studio Code</span>
                                </div>
                            </button>
                        );
                    })}
                </div>
            </div>
        </div>
    );
}
