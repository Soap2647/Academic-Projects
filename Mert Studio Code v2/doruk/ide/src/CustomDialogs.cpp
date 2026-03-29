#include "CustomDialogs.h"
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

CustomDialog::CustomDialog(const QString& title, QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Background frame for the rounded shadow look
    QWidget* bgWidget = new QWidget(this);
    bgWidget->setObjectName("dialogBg");
    bgWidget->setStyleSheet("QWidget#dialogBg { background-color: #1A1A24; border: 1px solid #333344; border-radius: 12px; }");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 160));
    shadow->setOffset(0, 5);
    bgWidget->setGraphicsEffect(shadow);

    QVBoxLayout* bgLayout = new QVBoxLayout(bgWidget);
    bgLayout->setContentsMargins(0, 0, 0, 0);
    bgLayout->setSpacing(0);

    // Title bar
    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(50);
    titleBar->setStyleSheet("border-bottom: 1px solid #333344;");
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(20, 0, 15, 0);

    QLabel* titleLabel = new QLabel(title, this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white; border: none;");
    
    QPushButton* closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(30, 30);
    closeBtn->setStyleSheet("QPushButton { background: transparent; color: #888; border: none; font-size: 14px; font-weight: bold; } QPushButton:hover { color: #fff; background: rgba(255,255,255,0.1); border-radius: 15px; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeBtn);

    bgLayout->addWidget(titleBar);

    // Content Area
    QWidget* contentWidget = new QWidget(this);
    contentWidget->setStyleSheet("border: none; background: transparent;");
    contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(10);

    bgLayout->addWidget(contentWidget);
    mainLayout->addWidget(bgWidget);
}

void CustomDialog::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void CustomDialog::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && !m_dragPosition.isNull()) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

// ---------------------------------------------------------
// SettingsDialog
// ---------------------------------------------------------

#include "CodeEditor.h"

SettingsDialog::SettingsDialog(CodeEditor* editor, QWidget *parent) 
    : CustomDialog("⚙ Ayarlar", parent), m_editor(editor) {
    resize(400, 500);
    
    QPushButton* fontMinus;
    QPushButton* fontPlus;
    QLabel* fontLabel;
    contentLayout->addWidget(createSpinBoxRow("T", "Yazı Boyutu", m_editor->font().pointSize(), &fontMinus, &fontPlus, &fontLabel));
    
    connect(fontMinus, &QPushButton::clicked, this, [this, fontLabel]() {
        int s = m_editor->font().pointSize();
        if (s > 6) { s--; QFont f = m_editor->font(); f.setPointSize(s); m_editor->setFont(f); fontLabel->setText(QString::number(s)); }
    });
    connect(fontPlus, &QPushButton::clicked, this, [this, fontLabel]() {
        int s = m_editor->font().pointSize();
        if (s < 48) { s++; QFont f = m_editor->font(); f.setPointSize(s); m_editor->setFont(f); fontLabel->setText(QString::number(s)); }
    });

    QPushButton* wordWrapToggle;
    bool wrapOn = m_editor->lineWrapMode() == QPlainTextEdit::WidgetWidth;
    contentLayout->addWidget(createToggleRow("↩", "Sözcük Kaydırma", wrapOn, &wordWrapToggle));
    connect(wordWrapToggle, &QPushButton::toggled, this, [this](bool checked) {
        m_editor->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    });

    QPushButton* dummy1;
    contentLayout->addWidget(createToggleRow("1 2 3", "Satır Numaraları", true, &dummy1));
    QPushButton* dummy2;
    contentLayout->addWidget(createToggleRow("🗺", "Mini Harita", true, &dummy2));
    QPushButton* dummy3;
    contentLayout->addWidget(createToggleRow("💾", "Otomatik Kaydet (Local)", true, &dummy3));
    
    contentLayout->addStretch();
}

QWidget* SettingsDialog::createToggleRow(const QString& iconText, const QString& labelText, bool defaultValue, QPushButton** outToggle) {
    QWidget* row = new QWidget(this);
    row->setFixedHeight(45);
    row->setStyleSheet("QWidget { background: rgba(255,255,255,0.03); border-radius: 8px; } QWidget:hover { background: rgba(255,255,255,0.06); }");
    
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(15, 5, 15, 5);
    
    QLabel* icon = new QLabel(iconText, this);
    icon->setFixedSize(24, 24);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: transparent; color: #aaa; border: 1px solid #444; border-radius: 4px; font-size: 12px;");
    
    QLabel* label = new QLabel(labelText, this);
    label->setStyleSheet("background: transparent; color: #ddd; font-weight: bold; font-size: 13px;");
    
    // Fake CSS toggle switch
    QPushButton* toggle = new QPushButton(this);
    toggle->setFixedSize(40, 20);
    toggle->setCheckable(true);
    toggle->setChecked(defaultValue);
    
    QString toggleStyle = "QPushButton { border: none; border-radius: 10px; }"
                          "QPushButton:checked { background-color: #38bdf8; }"
                          "QPushButton:!checked { background-color: #444; }";
    toggle->setStyleSheet(toggleStyle);
    
    if (outToggle) *outToggle = toggle;
    
    layout->addWidget(icon);
    layout->addSpacing(10);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(toggle);
    
    return row;
}

QWidget* SettingsDialog::createSpinBoxRow(const QString& iconText, const QString& labelText, int value, QPushButton** outMinus, QPushButton** outPlus, QLabel** outLabel) {
    QWidget* row = new QWidget(this);
    row->setFixedHeight(45);
    row->setStyleSheet("QWidget { background: rgba(255,255,255,0.03); border-radius: 8px; } QWidget:hover { background: rgba(255,255,255,0.06); }");
    
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(15, 5, 15, 5);
    
    QLabel* icon = new QLabel(iconText, this);
    icon->setFixedSize(24, 24);
    icon->setAlignment(Qt::AlignCenter);
    icon->setStyleSheet("background: transparent; color: #aaa; border: 1px solid #444; border-radius: 4px; font-size: 12px;");
    
    QLabel* label = new QLabel(labelText, this);
    label->setStyleSheet("background: transparent; color: #ddd; font-weight: bold; font-size: 13px;");
    
    QPushButton* minus = new QPushButton("-", this);
    minus->setFixedSize(24, 24);
    minus->setStyleSheet("background: #334; color: #fff; border: none; border-radius: 4px;");
    
    QLabel* valLabel = new QLabel(QString::number(value), this);
    valLabel->setAlignment(Qt::AlignCenter);
    valLabel->setStyleSheet("background: transparent; color: white; width: 30px; font-weight: bold;");
    
    QPushButton* plus = new QPushButton("+", this);
    plus->setFixedSize(24, 24);
    plus->setStyleSheet("background: #334; color: #fff; border: none; border-radius: 4px;");
    
    if (outMinus) *outMinus = minus;
    if (outPlus) *outPlus = plus;
    if (outLabel) *outLabel = valLabel;
    
    layout->addWidget(icon);
    layout->addSpacing(10);
    layout->addWidget(label);
    layout->addStretch();
    layout->addWidget(minus);
    layout->addWidget(valLabel);
    layout->addWidget(plus);
    
    return row;
}

// ---------------------------------------------------------
// ShortcutsDialog
// ---------------------------------------------------------

ShortcutsDialog::ShortcutsDialog(QWidget *parent) : CustomDialog("Klavye Kısayolları", parent) {
    resize(400, 500);
    
    contentLayout->addWidget(createShortcutRow("4 boşluk girintileme", "Tab"));
    contentLayout->addWidget(createShortcutRow("Otomatik girintileme", "Enter"));
    contentLayout->addWidget(createShortcutRow("Kod tamamlamayı aç", "Ctrl+Space"));
    contentLayout->addWidget(createShortcutRow("Geri al", "Ctrl+Z"));
    contentLayout->addWidget(createShortcutRow("İleri al", "Ctrl+Y"));
    contentLayout->addWidget(createShortcutRow("Satır kopyala", "Ctrl+D"));
    contentLayout->addWidget(createShortcutRow("Satır sil", "Ctrl+Shift+K"));
    contentLayout->addWidget(createShortcutRow("Satır taşı", "Alt+↑/↓"));
    contentLayout->addWidget(createShortcutRow("Bul & Değiştir", "Ctrl+F / Ctrl+H"));
    contentLayout->addWidget(createShortcutRow("Komut Paleti", "Ctrl+Shift+P"));
    contentLayout->addWidget(createShortcutRow("Tümünü seç", "Ctrl+A"));
    contentLayout->addWidget(createShortcutRow("Dosya yeniden adlandır", "Çift Tık"));
    
    contentLayout->addStretch();
}

QWidget* ShortcutsDialog::createShortcutRow(const QString& description, const QString& shortcutKeys) {
    QWidget* row = new QWidget(this);
    row->setFixedHeight(36);
    row->setStyleSheet("QWidget { background: transparent; border-bottom: 1px solid rgba(255,255,255,0.05); }");
    
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(10, 0, 10, 0);
    
    QLabel* desc = new QLabel(description, this);
    desc->setStyleSheet("color: #aaa; font-size: 12px; border: none;");
    
    QLabel* shortcut = new QLabel(shortcutKeys, this);
    shortcut->setAlignment(Qt::AlignCenter);
    // Keyboard key badge style
    shortcut->setStyleSheet("background: rgba(255,255,255,0.1); color: #ddd; border: 1px solid #445; border-radius: 4px; padding: 2px 8px; font-weight: bold; font-family: monospace; font-size: 11px;");
    
    layout->addWidget(desc);
    layout->addStretch();
    layout->addWidget(shortcut);
    
    return row;
}
