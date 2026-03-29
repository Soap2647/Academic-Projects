#pragma once

#include <QPlainTextEdit>
#include <QList>
#include <QCompleter>

struct CodeDiagnostic {
    int line;
    int col;
    int length;
    QString message;
};

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    // Theming interface
    void setLineHighlightColor(const QColor &color);
    void setGutterColors(const QColor &bg, const QColor &fg);
    void setDiagnostics(const QList<CodeDiagnostic>& diags);
    
    // Autocompletion
    void setCompleter(QCompleter* completer);
    QCompleter* completer() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void insertCompletion(const QString& completion);

private:
    QString textUnderCursor() const;
    QWidget *lineNumberArea;
    QColor m_lineHighlightColor;
    QColor m_gutterBackground;
    QColor m_gutterForeground;
    QList<CodeDiagnostic> m_diagnostics;
    QCompleter *c;
};

class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};
