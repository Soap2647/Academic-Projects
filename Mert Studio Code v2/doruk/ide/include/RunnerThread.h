#pragma once

#include <QThread>
#include <QString>
#include <QMutex>
#include <QWaitCondition>

class RunnerThread : public QThread {
    Q_OBJECT
public:
    explicit RunnerThread(const QString& sourceCode, const QString& filePath, QObject *parent = nullptr);
    ~RunnerThread() override;

    void stop();
    void provideInput(const QString& input);

signals:
    void outputReceived(const QString& text);
    void errorReceived(int line, int col, const QString& msg);
    void executionFinished(const QString& timeStr, bool success, int errorCount);

protected:
    void run() override;

private:
    QString m_sourceCode;
    QString m_filePath;
    
    QMutex m_mutex;
    QWaitCondition m_inputCondition;
    bool m_stopFlag;
    
    QString m_pendingInput;
    bool m_hasInput;
};
