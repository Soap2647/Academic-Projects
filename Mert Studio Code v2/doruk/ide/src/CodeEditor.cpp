#include "CodeEditor.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QToolTip>
#include <QAbstractItemView>
#include <QScrollBar>

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent), c(nullptr)
{
    lineNumberArea = new LineNumberArea(this);

    // Styling defaults
    m_lineHighlightColor = QColor(Qt::yellow).lighter(160);
    m_gutterBackground = QColor(240, 240, 240);
    m_gutterForeground = Qt::black;

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // Tab = 4 spaces configuration
    const int tabStop = 4 * fontMetrics().horizontalAdvance(' ');
    setTabStopDistance(tabStop);

    setMouseTracking(true); // For tooltips
}

void CodeEditor::setCompleter(QCompleter *completer)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, QOverload<const QString &>::of(&QCompleter::activated),
                     this, &CodeEditor::insertCompletion);
}

QCompleter *CodeEditor::completer() const
{
    return c;
}

void CodeEditor::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::setDiagnostics(const QList<CodeDiagnostic>& diags) {
    m_diagnostics = diags;
    highlightCurrentLine(); // Renders extra selections
    lineNumberArea->update();
}

void CodeEditor::setLineHighlightColor(const QColor &color) {
    m_lineHighlightColor = color;
    highlightCurrentLine();
}

void CodeEditor::setGutterColors(const QColor &bg, const QColor &fg) {
    m_gutterBackground = bg;
    m_gutterForeground = fg;
    lineNumberArea->update();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // Add space for error icons later if needed
    int space = 24 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_lineHighlightColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    
    // Add red squiggles for errors
    for (const auto& diag : m_diagnostics) {
        QTextEdit::ExtraSelection errSel;
        QTextCursor cursor(document());
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, diag.line - 1);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, diag.col - 1);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 5); // Approximate length if not provided
        
        errSel.cursor = cursor;
        errSel.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        errSel.format.setUnderlineColor(Qt::red);
        errSel.format.clearBackground();
        extraSelections.append(errSel);
    }

    setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), m_gutterBackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(m_gutterForeground);
            painter.drawText(0, top, lineNumberArea->width() - 8, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
                             
            // Check if line has error
            for (const auto& diag : m_diagnostics) {
                if (diag.line == blockNumber + 1) {
                    painter.setPen(Qt::red);
                    painter.setBrush(Qt::red);
                    painter.drawEllipse(2, top + (fontMetrics().height() - 8) / 2, 8, 8);
                    break;
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::mouseMoveEvent(QMouseEvent *e) {
    QPlainTextEdit::mouseMoveEvent(e);
    
    QTextCursor cursor = cursorForPosition(e->pos());
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock() + 1;
    
    for (const auto& diag : m_diagnostics) {
        if (diag.line == line && col >= diag.col && col <= diag.col + 5) {
            QToolTip::showText(e->globalPosition().toPoint(), diag.message, this);
            return;
        }
    }
    QToolTip::hideText();
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
        default:
            break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);
    // Auto-indent
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        QPlainTextEdit::keyPressEvent(e);
        QTextCursor cursor = textCursor();
        QTextBlock prevBlock = cursor.block().previous();
        QString prevText = prevBlock.text();
        
        QString indent = "";
        for (int i = 0; i < prevText.length(); ++i) {
            if (prevText[i].isSpace()) {
                indent += prevText[i];
            } else {
                break;
            }
        }
        
        if (prevText.endsWith('{')) {
            indent += "    ";
        }
        
        cursor.insertText(indent);
        return;
    }

    // Auto-pair
    if (e->key() == Qt::Key_ParenLeft || e->key() == Qt::Key_BraceLeft || 
        e->key() == Qt::Key_BracketLeft || e->text() == "\"" || e->text() == "'") {
        
        QPlainTextEdit::keyPressEvent(e);
        QTextCursor cursor = textCursor();
        
        QChar pairStr;
        if (e->key() == Qt::Key_ParenLeft) pairStr = ')';
        else if (e->key() == Qt::Key_BraceLeft) pairStr = '}';
        else if (e->key() == Qt::Key_BracketLeft) pairStr = ']';
        else if (e->text() == "\"") pairStr = '"';
        else if (e->text() == "'") pairStr = '\'';
        
        cursor.insertText(pairStr);
        cursor.movePosition(QTextCursor::Left);
        setTextCursor(cursor);
        return;
    }

    // Tab = 4 spaces
    if (e->key() == Qt::Key_Tab && !isShortcut) {
        textCursor().insertText("    ");
        return;
    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}

