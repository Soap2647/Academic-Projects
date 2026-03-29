#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class CustomDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomDialog(const QString& title, QWidget *parent = nullptr);

protected:
    void mousePressEvent(class QMouseEvent *event) override;
    void mouseMoveEvent(class QMouseEvent *event) override;

    QVBoxLayout* contentLayout;

private:
    class QPoint m_dragPosition;
};

class CodeEditor; // Forward declaration

class SettingsDialog : public CustomDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(CodeEditor* editor, QWidget *parent = nullptr);

private:
    QWidget* createToggleRow(const QString& iconText, const QString& labelText, bool defaultValue, QPushButton** outToggle);
    QWidget* createSpinBoxRow(const QString& iconText, const QString& labelText, int value, QPushButton** outMinus, QPushButton** outPlus, class QLabel** outLabel);
    
    CodeEditor* m_editor;
};

class ShortcutsDialog : public CustomDialog {
    Q_OBJECT
public:
    explicit ShortcutsDialog(QWidget *parent = nullptr);

private:
    QWidget* createShortcutRow(const QString& description, const QString& shortcutKeys);
};
