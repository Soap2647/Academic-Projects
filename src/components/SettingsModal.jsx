import { X, Settings, Type, ListOrdered, Map as MapIcon, Save, WrapText, AlignLeft } from 'lucide-react';
import { themes } from './editorUtils';

export default function SettingsModal({ theme, onClose, settings, onUpdateSettings }) {
    const t = themes[theme] || themes.dark;

    const Option = ({ label, icon: Icon, children }) => (
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '12px 0', borderBottom: `1px solid ${t.headerBorder}` }}>
            <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
                <div style={{ width: 32, height: 32, borderRadius: 8, background: t.buttonBg, display: 'flex', alignItems: 'center', justifyContent: 'center' }}>
                    <Icon size={16} style={{ color: t.textColor }} />
                </div>
                <span style={{ color: t.textColor, fontSize: 13, fontWeight: 500 }}>{label}</span>
            </div>
            {children}
        </div>
    );

    const Toggle = ({ checked, onChange }) => (
        <button onClick={() => onChange(!checked)} style={{
            width: 40, height: 22, borderRadius: 12, border: 'none', cursor: 'pointer',
            background: checked ? t.keyword : t.buttonBg, position: 'relative', transition: 'background 0.2s',
        }}>
            <div style={{
                width: 16, height: 16, borderRadius: '50%', background: '#fff',
                position: 'absolute', top: 3, left: checked ? 21 : 3, transition: 'left 0.2s',
            }} />
        </button>
    );

    return (
        <div style={{
            position: 'fixed', inset: 0, zIndex: 200, display: 'flex', alignItems: 'center', justifyContent: 'center',
            background: 'rgba(0,0,0,0.6)', backdropFilter: 'blur(8px)', animation: 'fadeIn 0.2s ease-out',
        }} onClick={onClose}>
            <div onClick={(e) => e.stopPropagation()} style={{
                background: t.bg, borderRadius: 16, width: '100%', maxWidth: 420, overflow: 'hidden',
                boxShadow: '0 24px 80px rgba(0,0,0,0.5), 0 0 0 1px rgba(255,255,255,0.08)', animation: 'modalSlideIn 0.3s ease-out',
            }}>
                {/* Header */}
                <div style={{
                    display: 'flex', alignItems: 'center', justifyContent: 'space-between',
                    padding: '16px 24px', borderBottom: `1px solid ${t.headerBorder}`,
                }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: 10 }}>
                        <Settings size={20} style={{ color: t.keyword }} />
                        <span style={{ color: t.textColor, fontWeight: 700, fontSize: 16 }}>Ayarlar</span>
                    </div>
                    <button onClick={onClose} style={{
                        background: 'transparent', border: 'none', cursor: 'pointer', color: t.textMuted,
                    }}><X size={18} /></button>
                </div>

                {/* Body */}
                <div style={{ padding: '0 24px 24px' }}>

                    <Option label="Yazı Boyutu" icon={Type}>
                        <div style={{ display: 'flex', alignItems: 'center', gap: 8 }}>
                            <button onClick={() => onUpdateSettings('fontSize', Math.max(10, settings.fontSize - 1))}
                                style={{ background: t.buttonBg, border: 'none', color: t.textColor, width: 24, height: 24, borderRadius: 4, cursor: 'pointer' }}>-</button>
                            <span style={{ color: t.textColor, fontSize: 13, minWidth: 20, textAlign: 'center' }}>{settings.fontSize}</span>
                            <button onClick={() => onUpdateSettings('fontSize', Math.min(24, settings.fontSize + 1))}
                                style={{ background: t.buttonBg, border: 'none', color: t.textColor, width: 24, height: 24, borderRadius: 4, cursor: 'pointer' }}>+</button>
                        </div>
                    </Option>

                    <Option label="Satır Numaraları" icon={ListOrdered}>
                        <Toggle checked={settings.showLineNumbers} onChange={(v) => onUpdateSettings('showLineNumbers', v)} />
                    </Option>

                    <Option label="Mini Harita" icon={MapIcon}>
                        <Toggle checked={settings.showMinimap} onChange={(v) => onUpdateSettings('showMinimap', v)} />
                    </Option>

                    <Option label="Otomatik Kaydet (Local)" icon={Save}>
                        <Toggle checked={settings.autoSave} onChange={(v) => onUpdateSettings('autoSave', v)} />
                    </Option>

                    <Option label="Sözcük Kaydırma" icon={WrapText}>
                        <Toggle checked={settings.wordWrap} onChange={(v) => onUpdateSettings('wordWrap', v)} />
                    </Option>

                    <Option label="Girinti Çizgileri" icon={AlignLeft}>
                        <Toggle checked={settings.indentGuides} onChange={(v) => onUpdateSettings('indentGuides', v)} />
                    </Option>

                </div>
            </div>
        </div>
    );
}
