#include "MinimapWidget.h"
#include <QPainter>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QTextBlock>
#include <QScrollBar>

MinimapWidget::MinimapWidget(QPlainTextEdit* editor, QWidget *parent)
    : QWidget(parent), m_editor(editor), m_scale(0.2)
{
    setFixedWidth(80);
    setCursor(Qt::PointingHandCursor);
    
    // Connect signals for live updates
    connect(m_editor->document(), &QTextDocument::contentsChanged, this, &MinimapWidget::updateMinimap);
    connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, &MinimapWidget::updateMinimap);
    connect(m_editor, &QPlainTextEdit::updateRequest, this, &MinimapWidget::updateMinimap);
}

void MinimapWidget::updateMinimap()
{
    update();
}

void MinimapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Background
    painter.fillRect(rect(), QColor(26, 26, 36, 150)); // Slightly darker/transparent than editor background

    // Current viewport rectangle
    int currentScroll = m_editor->verticalScrollBar()->value();
    
    qreal documentHeight = m_editor->document()->size().height();
    qreal viewportHeight = m_editor->viewport()->height();
    
    // Map editor viewport to minimap coordinates
    qreal mapRatio = documentHeight > 0 ? (qreal)height() / documentHeight : 1.0;
    
    // Don't draw scaled text if map ratio is too squished (optional optimization)
    // Here we draw an approximation of the code
    painter.setPen(Qt::NoPen);
    
    for (QTextBlock block = m_editor->document()->begin(); block.isValid(); block = block.next()) {
        QRectF blockRect = m_editor->document()->documentLayout()->blockBoundingRect(block);
        
        // Draw the block as a simple colored line
        QString text = block.text().trimmed();
        if (text.isEmpty()) continue;
        
        qreal yPos = blockRect.top() * mapRatio;
        qreal h = qMax(1.0, blockRect.height() * mapRatio);
        
        // Use a generic code color for minimap lines
        QColor lineColor(120, 120, 150, 180);
        painter.fillRect(QRectF(4, yPos, qMin(72.0, text.length() * 1.5), h), lineColor);
    }

    // Highlight the currently visible area
    qreal visibleTop = currentScroll * mapRatio;
    qreal visibleHeight = viewportHeight * mapRatio;
    
    // Ensure the highlight box isn't bigger than the widget itself
    if (visibleTop + visibleHeight > height()) {
        visibleHeight = height() - visibleTop;
    }
    
    painter.fillRect(QRectF(0, visibleTop, width(), visibleHeight), QColor(255, 255, 255, 30));
    painter.setPen(QPen(QColor(255, 255, 255, 60), 1));
    painter.drawRect(QRectF(0, visibleTop, width() - 1, visibleHeight));
}

void MinimapWidget::scrollEditorTo(int yPos)
{
    if (!m_editor || !m_editor->document()) return;
    
    qreal mapRatio = (qreal)height() / m_editor->document()->size().height();
    if (mapRatio <= 0.0) return;
    
    // Calculate the target scroll value based on the click
    // Center the viewport on the click
    qreal viewportHeightMap = (m_editor->viewport()->height() * mapRatio);
    qreal targetY = yPos - (viewportHeightMap / 2.0);
    
    int scrollValue = (int)(targetY / mapRatio);
    
    m_editor->verticalScrollBar()->setValue(scrollValue);
    update();
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scrollEditorTo(event->pos().y());
    }
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        scrollEditorTo(event->pos().y());
    }
}
