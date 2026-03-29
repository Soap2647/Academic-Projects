#pragma once

#include <QString>
#include <QColor>
#include <QList>

struct ThemeConfig {
    QString name;
    
    // Editor
    QColor editorBg;
    QColor editorFg;
    
    // Gutter
    QColor gutterBg;
    QColor gutterFg;
    
    // Syntax
    QColor keyword;
    QColor stringColor;
    QColor number;
    QColor comment;
    QColor operatorColor;
    
    // Highlight
    QColor selection;
    QColor currentLine;
    
    // UI
    QColor panelBg;
    QColor panelBorder;
    QColor panelText;
    
    // Diagnostics
    QColor errorUnderline;
    QColor warningUnderline;
    
    // Accent
    QColor accent;
};

class ThemeManager {
public:
    ThemeManager();
    
    const QList<ThemeConfig>& getThemes() const;
    void applyTheme(int index, class MainWindow* window, class CodeEditor* editor, class DorukHighlighter* highlighter);
    
    int loadSavedThemeIndex();
    void saveThemeIndex(int index);

private:
    QList<ThemeConfig> themes;
};
