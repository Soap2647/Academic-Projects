#include "ThemeManager.h"
#include "CodeEditor.h"
#include "DorukHighlighter.h"
#include "MainWindow.h"
#include <QSettings>
#include <QFile>

ThemeManager::ThemeManager() {
    // 1. Gece Mavisi
    themes.append({
        "Gece Mavisi",
        QColor("#0f172a"), QColor("#e2e8f0"), // Editor
        QColor("#1e293b"), QColor("#64748b"), // Gutter
        QColor("#0ea5e9"), QColor("#34d399"), QColor("#f43f5e"), QColor("#64748b"), QColor("#cbd5e1"), // Syntax
        QColor("#38bdf8").lighter(130), QColor("#1e293b"), // Selection, Current Line
        QColor("#0f172a"), QColor("#334155"), QColor("#e2e8f0"), // UI
        QColor("#ef4444"), QColor("#eab308"), // Diagnostics
        QColor("#0ea5e9") // Accent
    });

    // 2. Çöl Fırtınası
    themes.append({
        "Çöl Fırtınası",
        QColor("#fdf6e3"), QColor("#657b83"),
        QColor("#eee8d5"), QColor("#93a1a1"),
        QColor("#cb4b16"), QColor("#2aa198"), QColor("#d33682"), QColor("#93a1a1"), QColor("#586e75"),
        QColor("#eee8d5").darker(110), QColor("#eee8d5"),
        QColor("#fdf6e3"), QColor("#eee8d5"), QColor("#657b83"),
        QColor("#dc322f"), QColor("#b58900"),
        QColor("#cb4b16")
    });

    // 3. Orman
    themes.append({
        "Orman",
        QColor("#142E1B"), QColor("#C3D9C6"),
        QColor("#1D4226"), QColor("#799E81"),
        QColor("#84E296"), QColor("#E2C044"), QColor("#F27D52"), QColor("#5E8A66"), QColor("#A5C4AB"),
        QColor("#84E296").darker(200), QColor("#1D4226"),
        QColor("#142E1B"), QColor("#2A5c3A"), QColor("#C3D9C6"),
        QColor("#F27D52"), QColor("#E2C044"),
        QColor("#84E296")
    });

    // 4. Kızıl Gezegen
    themes.append({
        "Kızıl Gezegen",
        QColor("#2B1216"), QColor("#E8C1C6"),
        QColor("#3A1B22"), QColor("#9E7A80"),
        QColor("#FF5A5F"), QColor("#F4A261"), QColor("#E76F51"), QColor("#8A646A"), QColor("#DAB1B7"),
        QColor("#FF5A5F").darker(200), QColor("#3A1B22"),
        QColor("#2B1216"), QColor("#4F242C"), QColor("#E8C1C6"),
        QColor("#FF2A2F"), QColor("#F4A261"),
        QColor("#FF5A5F")
    });

    // 5. Kutup
    themes.append({
        "Kutup",
        QColor("#FFFFFF"), QColor("#2D3748"),
        QColor("#F7FAFC"), QColor("#A0AEC0"),
        QColor("#2B6CB0"), QColor("#38A169"), QColor("#E53E3E"), QColor("#CBD5E0"), QColor("#4A5568"),
        QColor("#EBF8FF"), QColor("#F7FAFC"),
        QColor("#FFFFFF"), QColor("#E2E8F0"), QColor("#2D3748"),
        QColor("#E53E3E"), QColor("#DD6B20"),
        QColor("#2B6CB0")
    });

    // 6. Obsidyen
    themes.append({
        "Obsidyen",
        QColor("#0A0A0A"), QColor("#E0E0E0"),
        QColor("#141414"), QColor("#666666"),
        QColor("#FFD700"), QColor("#4CAF50"), QColor("#FF5252"), QColor("#808080"), QColor("#B0B0B0"),
        QColor("#333333"), QColor("#1A1A1A"),
        QColor("#0A0A0A"), QColor("#242424"), QColor("#E0E0E0"),
        QColor("#FF5252"), QColor("#FFC107"),
        QColor("#FFD700")
    });

    // 7. Retro Amber
    themes.append({
        "Retro Amber",
        QColor("#000000"), QColor("#FFB000"),
        QColor("#0A0A0A"), QColor("#805800"),
        QColor("#FFD000"), QColor("#FF8C00"), QColor("#FF4500"), QColor("#664600"), QColor("#CC8C00"),
        QColor("#332300"), QColor("#1A1100"),
        QColor("#000000"), QColor("#1A1100"), QColor("#FFB000"),
        QColor("#FF0000"), QColor("#FFD000"),
        QColor("#FFB000")
    });

    // 8. Lavanta
    themes.append({
        "Lavanta",
        QColor("#292836"), QColor("#E2E0EE"),
        QColor("#363447"), QColor("#8F8CAE"),
        QColor("#B388FF"), QColor("#69F0AE"), QColor("#FF8A80"), QColor("#6A6785"), QColor("#C3C1D6"),
        QColor("#453D68"), QColor("#363447"),
        QColor("#292836"), QColor("#46435C"), QColor("#E2E0EE"),
        QColor("#FF5252"), QColor("#FFD54F"),
        QColor("#B388FF")
    });

    // 9. Günbatımı
    themes.append({
        "Günbatımı",
        QColor("#1D1836"), QColor("#F4E7E7"),
        QColor("#2B234D"), QColor("#897C9E"),
        QColor("#FF7B89"), QColor("#F9CB84"), QColor("#62D8B6"), QColor("#64597F"), QColor("#D6CAE2"),
        QColor("#FF7B89").darker(250), QColor("#2B234D"),
        QColor("#1D1836"), QColor("#3F3366"), QColor("#F4E7E7"),
        QColor("#FF4A5C"), QColor("#F9CB84"),
        QColor("#FF7B89")
    });

    // 10. Pastel
    themes.append({
        "Pastel",
        QColor("#FDFDFD"), QColor("#4A4A4A"),
        QColor("#F4F4F4"), QColor("#A1A1A1"),
        QColor("#A0C4FF"), QColor("#BDB2FF"), QColor("#FFADAD"), QColor("#D3D3D3"), QColor("#707070"),
        QColor("#CAE0FF"), QColor("#F4F4F4"),
        QColor("#FDFDFD"), QColor("#E8E8E8"), QColor("#4A4A4A"),
        QColor("#FF6B6B"), QColor("#FFD93D"),
        QColor("#A0C4FF")
    });

    // 11. Yüksek Kontrast
    themes.append({
        "Yüksek Kontrast",
        QColor("#000000"), QColor("#FFFFFF"),
        QColor("#000000"), QColor("#FFFFFF"),
        QColor("#FFFF00"), QColor("#00FF00"), QColor("#00FFFF"), QColor("#AAAAAA"), QColor("#FFFFFF"),
        QColor("#000080"), QColor("#333333"),
        QColor("#000000"), QColor("#FFFFFF"), QColor("#FFFFFF"),
        QColor("#FF0000"), QColor("#FFFF00"),
        QColor("#FFFFFF")
    });

    // 12. Karbonlu
    themes.append({
        "Karbonlu",
        QColor("#161616"), QColor("#F4F4F4"),
        QColor("#262626"), QColor("#8D8D8D"),
        QColor("#08BDBA"), QColor("#42BE65"), QColor("#FF832B"), QColor("#525252"), QColor("#C6C6C6"),
        QColor("#08BDBA").darker(250), QColor("#262626"),
        QColor("#161616"), QColor("#393939"), QColor("#F4F4F4"),
        QColor("#FA4D56"), QColor("#F1C21B"),
        QColor("#08BDBA")
    });
}

const QList<ThemeConfig>& ThemeManager::getThemes() const {
    return themes;
}

int ThemeManager::loadSavedThemeIndex() {
    QSettings settings("DORUK", "IDE");
    return settings.value("themeIndex", 0).toInt();
}

void ThemeManager::saveThemeIndex(int index) {
    QSettings settings("DORUK", "IDE");
    settings.setValue("themeIndex", index);
}

void ThemeManager::applyTheme(int index, MainWindow* window, CodeEditor* editor, DorukHighlighter* highlighter) {
    if (index < 0 || index >= themes.size()) return;
    const ThemeConfig& t = themes[index];

    // Editor Styles
    QPalette p = editor->palette();
    p.setColor(QPalette::Base, t.editorBg);
    p.setColor(QPalette::Text, t.editorFg);
    p.setColor(QPalette::Highlight, t.selection);
    p.setColor(QPalette::HighlightedText, t.editorFg);
    editor->setPalette(p);
    
    editor->setGutterColors(t.gutterBg, t.gutterFg);
    editor->setLineHighlightColor(t.currentLine);

    // Highlighter Styles
    highlighter->setTheme(t.keyword, t.stringColor, t.number, t.comment, t.operatorColor);
    
    // Application wide StyleSheet
    QString css = QString(
        // Modern Reset
        "* { font-family: 'Segoe UI', 'San Francisco', sans-serif; }"
        
        // Window & Dialog
        "QMainWindow, QDialog { background-color: %1; color: %2; }"
        
        // Title Bar
        "#titleBar { background-color: %10; }"
        "#titleLabel { color: %2; }"
        "#windowControlBtn { background: transparent; color: %2; border: none; font-weight: bold; font-size: 14px; }"
        "#windowControlBtn:hover { background-color: rgba(255, 255, 255, 0.1); }"
        "#runButton { background-color: %6; color: white; border-radius: 4px; padding: 6px 16px; font-weight: bold; border: none; }"
        "#runButton:hover { background-color: rgba(255, 255, 255, 0.2); border: 1px solid %6; }"
        
        // Menu Bar
        "QMenuBar { background-color: transparent; color: %2; padding-top: 2px; }"
        "QMenuBar::item { padding: 5px 12px; background: transparent; border-radius: 4px; border: none; }"
        "QMenuBar::item:selected { background-color: rgba(255, 255, 255, 0.1); }"
        "QMenu { background-color: #1A1A24; color: #E0E0E0; border: 1px solid #333344; border-radius: 6px; padding: 4px; box-shadow: 0px 8px 16px rgba(0,0,0,0.5); }"
        "QMenu::item { padding: 6px 32px 6px 24px; border-radius: 4px; margin: 2px 4px; }"
        "QMenu::item:selected { background-color: %6; color: white; }"
        "QMenu::separator { height: 1px; background: #333344; margin: 4px 0px; }"
        
        // Activity Bar
        "#activityBar { background-color: %11; border-right: 1px solid %7; }"
        "#activityBtn { background: transparent; border: none; border-radius: 8px; margin: 0px 5px; color: #888; font-size: 16px; }"
        "#activityBtn:hover { background-color: rgba(255, 255, 255, 0.1); color: #fff; }"
        
        // Explorer Panel
        "#explorerPanel { background-color: %12; border-right: 1px solid %7; }"
        "#explorerTitle { font-weight: bold; font-size: 11px; color: %2; opacity: 0.5; }"
        "#fileTree { background: transparent; border: none; color: %2; outline: none; }"
        "#fileTree::item { padding: 6px; border-radius: 4px; margin: 0px 8px; }"
        "#fileTree::item:selected { background-color: %3; color: %2; }"
        "#fileTree::item:hover:!selected { background-color: rgba(255, 255, 255, 0.05); }"
        
        // Editor Tabs
        "#editorTabs::pane { border: none; background-color: %5; border-top: 1px solid %7; }"
        "QTabBar::tab { background-color: %4; color: %2; padding: 8px 24px; border-top-left-radius: 6px; border-top-right-radius: 6px; margin-right: 2px; border: none; }"
        "QTabBar::tab:selected { background-color: %5; color: %2; font-weight: bold; border-top: 2px solid %6; }"
        "QTabBar::tab:hover:!selected { background-color: rgba(255, 255, 255, 0.05); }"
        
        // Bottom Panel
        "#bottomPanel::pane { border: none; border-top: 1px solid %7; background-color: %12; }"
        "#terminalOutput, QPlainTextEdit, QListWidget { background-color: %12; color: %2; selection-background-color: %3; border: none; font-family: 'Consolas', monospace; }"
        
        // Splitters
        "QSplitter::handle { background-color: %7; }"
        
        // Scrollbars
        "QScrollBar:vertical { border: none; background: transparent; width: 12px; margin: 0px; }"
        "QScrollBar::handle:vertical { background-color: rgba(255, 255, 255, 0.15); border-radius: 6px; min-height: 20px; margin: 2px; }"
        "QScrollBar::handle:vertical:hover { background-color: rgba(255, 255, 255, 0.3); }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
        
        "QScrollBar:horizontal { border: none; background: transparent; height: 12px; margin: 0px; }"
        "QScrollBar::handle:horizontal { background-color: rgba(255, 255, 255, 0.15); border-radius: 6px; min-width: 20px; margin: 2px; }"
        "QScrollBar::handle:horizontal:hover { background-color: rgba(255, 255, 255, 0.3); }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }"
        
        // Status Bar
        "QStatusBar { background-color: #0078D7; color: white; border: none; font-size: 12px; }"
        "QStatusBar::item { border: none; }"
    )
    .arg(t.panelBg.name())        // 1
    .arg(t.panelText.name())      // 2
    .arg(t.selection.name())      // 3
    .arg(t.gutterBg.name())       // 4
    .arg(t.editorBg.name())       // 5
    .arg(t.accent.name())         // 6
    .arg(t.panelBorder.name())    // 7
    .arg("", "", "")              // 8, 9 dummy
    .arg(t.panelBg.darker(115).name()) // 10: Title Bar
    .arg(t.panelBg.darker(130).name()) // 11: Activity Bar / Status Bar
    .arg(t.panelBg.darker(105).name());// 12: Explorer Panel / Terminal

    window->setStyleSheet(css);
    
    saveThemeIndex(index);
}

