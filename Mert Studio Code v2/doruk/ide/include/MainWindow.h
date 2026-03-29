#pragma once

#include <QMainWindow>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTabWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
   void setupUi();
   void setupMenus();

private slots:
   void runProject();
   void stopProject();
   void clearPanels();
   void handleOutput(const QString& text);
   void handleError(int line, int col, const QString& msg);
   void handleExecutionFinished(const QString& timeStr, bool success, int errorCount);
   void handleProblemClicked(class QListWidgetItem* item);

private:
   class CodeEditor* editor;
   class DorukHighlighter* highlighter;
   class RunnerThread* runnerThread;
   class ThemeManager* themeManager;
   
   // UI Elements
   class QWidget* titleBar;
   class QWidget* activityBar;
   class QWidget* explorerPanel;
   
   QSplitter* mainSplitter;
   QTabWidget* bottomPanel;
   QPlainTextEdit* outputPanel;
   class QListWidget* problemsPanel;
   QPlainTextEdit* consolePanel;

   // Status bar labels
   class QLabel* statusCursorLabel;
   class QLabel* statusEncodingLabel;
   class QLabel* statusLangLabel;
   class QLabel* statusAppLabel;

   // Search & Replace UI
   class QWidget* searchPanel;
   class QLineEdit* searchInput;
   class QLineEdit* replaceInput;
   
   void toggleSearchPanel();
   void performSearch();
   void performReplace();
   void performReplaceAll();

   QList<struct CodeDiagnostic> m_currentDiagnostics;
   
   // Frameless Window Dragging
   class QPoint m_dragPosition;
};
