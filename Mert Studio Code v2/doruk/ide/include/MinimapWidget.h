#pragma once

#include <QWidget>
#include <QPlainTextEdit>

class MinimapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MinimapWidget(QPlainTextEdit* editor, QWidget *parent = nullptr);

public slots:
    void updateMinimap();

protected:
    void paintEvent(class QPaintEvent *event) override;
    void mousePressEvent(class QMouseEvent *event) override;
    void mouseMoveEvent(class QMouseEvent *event) override;

private:
    void scrollEditorTo(int yPos);

    QPlainTextEdit* m_editor;
    qreal m_scale;
};
