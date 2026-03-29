#include "MainWindow.h"
#include "CodeEditor.h"
#include "DorukHighlighter.h"
#include "RunnerThread.h"
#include "ThemeManager.h"
#include "CustomDialogs.h"
#include <QMenuBar>
#include <QListWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QStringListModel>
#include <QCompleter>
#include <QMouseEvent>
#include <QShortcut>
#include "MinimapWidget.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), runnerThread(nullptr) {
    setWindowTitle("Mert Studio Code \xE2\x80\x94 T\xC3\xBCrk\xC3\xA7\x65 Kod Edit\xC3\xB6r\xC3\xBC");
    resize(1200, 800);
    themeManager = new ThemeManager();
    setupUi();
    setupMenus();
    
    // Apply initial theme
    int savedTheme = themeManager->loadSavedThemeIndex();
    themeManager->applyTheme(savedTheme, this, editor, highlighter);
}

MainWindow::~MainWindow() {
    if (runnerThread) {
        runnerThread->stop();
        runnerThread->wait();
    }
}

void MainWindow::setupUi() {
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("mainContentWidget");
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- TITLE BAR ---
    titleBar = new QWidget(this);
    titleBar->setObjectName("titleBar");
    titleBar->setFixedHeight(40);
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(15, 0, 15, 0);

    QLabel* titleLabel = new QLabel("</> Mert Studio Code", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    titleLayout->addWidget(titleLabel);

    // We will attach the QMenuBar to the title layout in setupMenus.
    titleLayout->addSpacing(20);

    // QMenuBar
    QMenuBar* appMenuBar = menuBar();
    titleLayout->addWidget(appMenuBar);
    titleLayout->addStretch();

    QPushButton* runButton = new QPushButton("▶ Çalıştır", this);
    runButton->setObjectName("runButton");
    runButton->setCursor(Qt::PointingHandCursor);
    connect(runButton, &QPushButton::clicked, this, &MainWindow::runProject);
    titleLayout->addWidget(runButton);

    mainLayout->addWidget(titleBar);

    // --- CONTENT AREA ---
    QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal, this);
    horizontalSplitter->setObjectName("mainHorizontalSplitter");
    
    // Left Container (Activity + Explorer)
    QWidget* leftContainer = new QWidget(this);
    QHBoxLayout* leftLayout = new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(0);

    // Activity Bar
    activityBar = new QWidget(this);
    activityBar->setObjectName("activityBar");
    activityBar->setFixedWidth(50);
    QVBoxLayout* activityLayout = new QVBoxLayout(activityBar);
    activityLayout->setContentsMargins(0,10,0,0);
    activityLayout->setSpacing(15);
    activityLayout->setAlignment(Qt::AlignTop);

    QPushButton* filesBtn = new QPushButton(this);
    filesBtn->setObjectName("activityBtn");
    filesBtn->setIcon(QIcon::fromTheme("document-new")); // Placeholder
    filesBtn->setFixedSize(40, 40);
    activityLayout->addWidget(filesBtn);
    
    activityLayout->addStretch(); // Push settings to bottom
    
    QPushButton* shortcutsBtn = new QPushButton("⌨", this);
    shortcutsBtn->setObjectName("activityBtn");
    shortcutsBtn->setFixedSize(40, 40);
    connect(shortcutsBtn, &QPushButton::clicked, this, [this]() {
        ShortcutsDialog d(this);
        d.exec();
    });
    activityLayout->addWidget(shortcutsBtn);

    QPushButton* settingsBtn = new QPushButton("⚙", this);
    settingsBtn->setObjectName("activityBtn");
    settingsBtn->setFixedSize(40, 40);
    connect(settingsBtn, &QPushButton::clicked, this, [this]() {
        SettingsDialog d(editor, this);
        d.exec();
    });
    activityLayout->addWidget(settingsBtn);
    activityLayout->addSpacing(10);
    
    // Explorer Panel
    explorerPanel = new QWidget(this);
    explorerPanel->setObjectName("explorerPanel");
    QVBoxLayout* explorerLayout = new QVBoxLayout(explorerPanel);
    explorerLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* explorerTitle = new QLabel("DOSYALAR", this);
    explorerTitle->setObjectName("explorerTitle");
    explorerTitle->setContentsMargins(15, 10, 10, 10);
    
    QListWidget* fileTree = new QListWidget(this);
    fileTree->setObjectName("fileTree");
    fileTree->addItem("TR dosya_10.c.tr");
    
    explorerLayout->addWidget(explorerTitle);
    explorerLayout->addWidget(fileTree);

    leftLayout->addWidget(activityBar);
    leftLayout->addWidget(explorerPanel);

    horizontalSplitter->addWidget(leftContainer);

    // --- EDITOR AREA ---
    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->setObjectName("editorSplitter");

    // Tab widget for editor
    QTabWidget* editorTabs = new QTabWidget(this);
    editorTabs->setObjectName("editorTabs");
    editorTabs->setTabsClosable(true);

    editor = new CodeEditor(this);
    highlighter = new DorukHighlighter(editor->document());
    QFont font("Consolas", 11);
    editor->setFont(font);
    
    // Auto-completer setup
    QCompleter *completer = new QCompleter(this);
    QStringList keywords;
    keywords << "değişken" << "eğer" << "değilse" << "döngü" << "için" 
             << "fonksiyon" << "döndür" << "yazdır" << "oku" << "doğru" 
             << "yanlış" << "boş" << "ve" << "veya" << "değil" 
             << "kır" << "devam" << "sınıf" << "yeni" << "bu";
    
    QAbstractItemModel *model = new QStringListModel(keywords, completer);
    completer->setModel(model);
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    completer->popup()->setStyleSheet("QListView { background-color: #1A1A24; color: #E0E0E0; border: 1px solid #333344; border-radius: 4px; } QListView::item:selected { background-color: #38bdf8; color: white; }");
    
    editor->setCompleter(completer);

    // Text Editor + Minimap Container
    QWidget* editorContainer = new QWidget(this);
    QHBoxLayout* editorLayout = new QHBoxLayout(editorContainer);
    // Search Panel (Overlay or Top)
    searchPanel = new QWidget(editorContainer);
    searchPanel->setObjectName("searchPanel");
    searchPanel->setStyleSheet("QWidget#searchPanel { background: #1A1A24; border: 1px solid #333344; border-radius: 6px; }");
    searchPanel->hide(); // Hidden by default
    
    QGridLayout* searchLayout = new QGridLayout(searchPanel);
    searchLayout->setContentsMargins(8, 8, 8, 8);
    searchLayout->setSpacing(5);
    
    searchInput = new QLineEdit(searchPanel);
    searchInput->setPlaceholderText("Bul...");
    searchInput->setStyleSheet("background: #0D0D14; color: white; border: 1px solid #333344; padding: 4px; border-radius: 3px;");
    
    replaceInput = new QLineEdit(searchPanel);
    replaceInput->setPlaceholderText("Değiştir...");
    replaceInput->setStyleSheet("background: #0D0D14; color: white; border: 1px solid #333344; padding: 4px; border-radius: 3px;");
    
    QPushButton* nextBtn = new QPushButton("↓", searchPanel);
    QPushButton* prevBtn = new QPushButton("↑", searchPanel);
    QPushButton* replaceBtn = new QPushButton("Değiştir", searchPanel);
    QPushButton* replaceAllBtn = new QPushButton("Tümünü Değiştir", searchPanel);
    QPushButton* closeSearchBtn = new QPushButton("✕", searchPanel);
    
    nextBtn->setFixedSize(24, 24);
    prevBtn->setFixedSize(24, 24);
    closeSearchBtn->setFixedSize(24, 24);
    
    QString btnStyle = "QPushButton { background: transparent; color: #aaa; border: none; } QPushButton:hover { background: rgba(255,255,255,0.1); color: white; border-radius: 3px; }";
    nextBtn->setStyleSheet(btnStyle);
    prevBtn->setStyleSheet(btnStyle);
    closeSearchBtn->setStyleSheet(btnStyle);
    replaceBtn->setStyleSheet("background: #38bdf8; color: white; border: none; padding: 4px 8px; border-radius: 3px;");
    replaceAllBtn->setStyleSheet("background: rgba(255,255,255,0.1); color: white; border: none; padding: 4px 8px; border-radius: 3px;");
    
    searchLayout->addWidget(searchInput, 0, 0, 1, 2);
    searchLayout->addWidget(prevBtn, 0, 2);
    searchLayout->addWidget(nextBtn, 0, 3);
    searchLayout->addWidget(closeSearchBtn, 0, 4);
    
    searchLayout->addWidget(replaceInput, 1, 0, 1, 2);
    searchLayout->addWidget(replaceBtn, 1, 2, 1, 2);
    searchLayout->addWidget(replaceAllBtn, 1, 4);
    
    connect(nextBtn, &QPushButton::clicked, this, &MainWindow::performSearch);
    connect(searchInput, &QLineEdit::returnPressed, this, &MainWindow::performSearch);
    connect(replaceBtn, &QPushButton::clicked, this, &MainWindow::performReplace);
    connect(replaceAllBtn, &QPushButton::clicked, this, &MainWindow::performReplaceAll);
    connect(closeSearchBtn, &QPushButton::clicked, this, [this](){ searchPanel->hide(); });
    
    // Ctrl+F / Ctrl+H Shortcuts
    new QShortcut(QKeySequence("Ctrl+F"), this, SLOT(toggleSearchPanel()));
    new QShortcut(QKeySequence("Ctrl+H"), this, SLOT(toggleSearchPanel()));
    
    editorLayout->addWidget(editor);
    
    MinimapWidget* minimap = new MinimapWidget(editor, this);
    editorLayout->addWidget(minimap);

    editorTabs->addTab(editorContainer, "dosya_10.c.tr");

    bottomPanel = new QTabWidget(this);
    bottomPanel->setObjectName("bottomPanel");
    
    outputPanel = new QPlainTextEdit(this);
    outputPanel->setReadOnly(true);
    outputPanel->setObjectName("terminalOutput");
    
    problemsPanel = new QListWidget(this);
    connect(problemsPanel, &QListWidget::itemClicked, this, &MainWindow::handleProblemClicked);
    
    consolePanel = new QPlainTextEdit(this);

    bottomPanel->addTab(outputPanel, ">_ TERMİNAL");
    bottomPanel->addTab(problemsPanel, "⚠ Sorunlar");
    bottomPanel->addTab(consolePanel, "Konsol");

    mainSplitter->addWidget(editorTabs);
    mainSplitter->addWidget(bottomPanel);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 1);

    horizontalSplitter->addWidget(mainSplitter);
    horizontalSplitter->setStretchFactor(0, 1);
    horizontalSplitter->setStretchFactor(1, 6);
    horizontalSplitter->setCollapsible(0, false);
    
    mainLayout->addWidget(horizontalSplitter);
    setCentralWidget(centralWidget);

    // --- STATUS BAR ---
    QStatusBar* sBar = statusBar();
    
    statusCursorLabel = new QLabel("Satır 1, Sütun 1", this);
    statusCursorLabel->setContentsMargins(10, 0, 10, 0);
    sBar->addWidget(statusCursorLabel);
    
    statusAppLabel = new QLabel("Mert Studio Code", this);
    statusLangLabel = new QLabel("A  Türkçe C ∨", this);
    statusEncodingLabel = new QLabel("UTF-8", this);
    
    statusAppLabel->setContentsMargins(10, 0, 10, 0);
    statusLangLabel->setContentsMargins(10, 0, 10, 0);
    statusEncodingLabel->setContentsMargins(10, 0, 10, 0);
    
    sBar->addPermanentWidget(statusEncodingLabel);
    sBar->addPermanentWidget(statusLangLabel);
    sBar->addPermanentWidget(statusAppLabel);
    
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, [this]() {
        QTextCursor cursor = editor->textCursor();
        int line = cursor.blockNumber() + 1;
        int col = cursor.columnNumber() + 1;
        statusCursorLabel->setText(QString("Satır %1, Sütun %2").arg(line).arg(col));
    });
}

void MainWindow::toggleSearchPanel() {
    if (searchPanel->isVisible()) {
        searchPanel->hide();
        editor->setFocus();
    } else {
        // Position it at the top right of the editor
        int xPos = editor->width() - 320 - 30; // 320 width approx, 30 padding
        searchPanel->setGeometry(xPos, 10, 320, 80);
        searchPanel->show();
        searchPanel->raise();
        searchInput->setFocus();
        searchInput->selectAll();
    }
}

void MainWindow::performSearch() {
    QString query = searchInput->text();
    if (query.isEmpty()) return;
    
    bool found = editor->find(query);
    if (!found) {
        // Wrap around
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::Start);
        editor->setTextCursor(cursor);
        editor->find(query);
    }
}

void MainWindow::performReplace() {
    QString query = searchInput->text();
    if (query.isEmpty() || !editor->textCursor().hasSelection()) return;
    
    if (editor->textCursor().selectedText() == query) {
        editor->textCursor().insertText(replaceInput->text());
    }
    performSearch();
}

void MainWindow::performReplaceAll() {
    QString query = searchInput->text();
    if (query.isEmpty()) return;
    
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();
    
    cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(cursor);
    
    int count = 0;
    while (editor->find(query)) {
        editor->textCursor().insertText(replaceInput->text());
        count++;
    }
    
    cursor.endEditBlock();
    statusBar()->showMessage(QString("%1 değişiklik yapıldı.").arg(count), 3000);
}

void MainWindow::setupMenus() {
    // File
    QMenu* fileMenu = menuBar()->addMenu("Dosya");
    fileMenu->addAction("Yeni");
    fileMenu->addAction("Aç");
    fileMenu->addAction("Kaydet");
    fileMenu->addAction("Farklı Kaydet");
    fileMenu->addSeparator();
    fileMenu->addAction("Çıkış", this, &QWidget::close);

    // Edit
    QMenu* editMenu = menuBar()->addMenu("Düzenle");
    editMenu->addAction("Geri Al", editor, &QPlainTextEdit::undo);
    editMenu->addAction("Yinele", editor, &QPlainTextEdit::redo);
    editMenu->addSeparator();
    editMenu->addAction("Kes", editor, &QPlainTextEdit::cut);
    editMenu->addAction("Kopyala", editor, &QPlainTextEdit::copy);
    editMenu->addAction("Yapıştır", editor, &QPlainTextEdit::paste);
    editMenu->addSeparator();
    editMenu->addAction("Tümünü Seç", editor, &QPlainTextEdit::selectAll);

    // Run
    QMenu* runMenu = menuBar()->addMenu("Çalıştır");
    runMenu->addAction("Çalıştır (F5)", this, &MainWindow::runProject, QKeySequence(Qt::Key_F5));
    runMenu->addAction("Durdur", this, &MainWindow::stopProject);
    runMenu->addAction("Temizle", this, &MainWindow::clearPanels);

    // View
    QMenu* viewMenu = menuBar()->addMenu("Görünüm");
    QMenu* themeMenu = viewMenu->addMenu("Tema Seç");

    const QList<ThemeConfig>& themes = themeManager->getThemes();
    for (int i = 0; i < themes.size(); ++i) {
        QAction* themeAction = themeMenu->addAction(themes[i].name);
        connect(themeAction, &QAction::triggered, this, [this, i]() {
            themeManager->applyTheme(i, this, editor, highlighter);
        });
    }

    // About
    QMenu* aboutMenu = menuBar()->addMenu("Hakkında");
    aboutMenu->addAction("DORUK Hakkında", this, [this]() {
        QMessageBox::about(this, "DORUK Hakkında", "DORUK Programlama Dili IDE\nSürüm 1.0.0");
    });
}

