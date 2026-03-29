#include "MainWindow.h"
#include "RunnerThread.h"
#include "CodeEditor.h"
#include <QDateTime>
#include <QScrollBar>
#include <QListWidget>
#include <QStatusBar>

// Helper to append text to QPlainTextEdit without causing extra spaces
void MainWindow::handleOutput(const QString& text) {
    QTextCursor cursor(outputPanel->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text + "\n");
    outputPanel->setTextCursor(cursor);
    outputPanel->verticalScrollBar()->setValue(outputPanel->verticalScrollBar()->maximum());
}

void MainWindow::handleError(int line, int col, const QString& msg) {
    QString errorMsg = QString("[Satır %1, Sütun %2] HATA: %3").arg(line).arg(col).arg(msg);
    QListWidgetItem* item = new QListWidgetItem(errorMsg, problemsPanel);
    item->setData(Qt::UserRole, line);
    item->setData(Qt::UserRole + 1, col);

    CodeDiagnostic diag;
    diag.line = line;
    diag.col = col;
    diag.length = 5;
    diag.message = msg;
    m_currentDiagnostics.append(diag);
}

void MainWindow::handleExecutionFinished(const QString& timeStr, bool success, int errorCount) {
    editor->setDiagnostics(m_currentDiagnostics);

    if (success) {
        statusBar()->showMessage(QString("Tamamlandı (%1s)").arg(timeStr));
    } else {
        statusBar()->showMessage(QString("Hata (%1 sorun)").arg(errorCount));
    }
    
    if (runnerThread) {
        runnerThread->deleteLater();
        runnerThread = nullptr;
    }
}

void MainWindow::runProject() {
    if (runnerThread && runnerThread->isRunning()) {
        statusBar()->showMessage("Zaten çalışıyor...");
        return;
    }

    clearPanels();
    statusBar()->showMessage("Çalışıyor...");
    bottomPanel->setCurrentWidget(outputPanel);
    
    m_currentDiagnostics.clear();
    editor->setDiagnostics(m_currentDiagnostics);

    QString sourceCode = editor->toPlainText();
    // Simulate filename for now
    QString filePath = "dosya.drk";

    runnerThread = new RunnerThread(sourceCode, filePath, this);
    connect(runnerThread, &RunnerThread::outputReceived, this, &MainWindow::handleOutput);
    connect(runnerThread, &RunnerThread::errorReceived, this, &MainWindow::handleError);
    connect(runnerThread, &RunnerThread::executionFinished, this, &MainWindow::handleExecutionFinished);
    
    runnerThread->start();
}

void MainWindow::stopProject() {
    if (runnerThread && runnerThread->isRunning()) {
        runnerThread->stop();
        statusBar()->showMessage("Durduruldu.");
    }
}

void MainWindow::clearPanels() {
    outputPanel->clear();
    problemsPanel->clear();
    consolePanel->clear();
}

void MainWindow::handleProblemClicked(QListWidgetItem* item) {
    if (!item) return;

    int line = item->data(Qt::UserRole).toInt();
    int col = item->data(Qt::UserRole + 1).toInt();

    QTextCursor cursor(editor->document());
    cursor.setPosition(0);
    cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line - 1);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col - 1);
    
    editor->setTextCursor(cursor);
    editor->setFocus();
}

