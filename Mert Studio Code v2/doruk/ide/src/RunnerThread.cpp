#include "RunnerThread.h"
#include "Lexer.h"
#include "Parser.h"
#include "SemanticAnalyzer.h"
#include "Interpreter.h"

#include <QElapsedTimer>

RunnerThread::RunnerThread(const QString& sourceCode, const QString& filePath, QObject *parent)
    : QThread(parent)
    , m_sourceCode(sourceCode)
    , m_filePath(filePath)
    , m_stopFlag(false)
    , m_hasInput(false)
{
}

RunnerThread::~RunnerThread() {
    stop();
    wait();
}

void RunnerThread::stop() {
    QMutexLocker locker(&m_mutex);
    m_stopFlag = true;
    m_inputCondition.wakeAll();
}

void RunnerThread::provideInput(const QString& input) {
    QMutexLocker locker(&m_mutex);
    m_pendingInput = input;
    m_hasInput = true;
    m_inputCondition.wakeAll();
}

void RunnerThread::run() {
    QElapsedTimer timer;
    timer.start();

    // Setup callbacks
    Doruk::Interpreter::CiktiCb ciktiCallback = [this](const std::string& str) {
        emit outputReceived(QString::fromStdString(str));
    };

    Doruk::Interpreter::GirisCb girisCallback = [this]() -> std::string {
        QMutexLocker locker(&m_mutex);
        while (!m_hasInput && !m_stopFlag) {
            m_inputCondition.wait(&m_mutex);
        }
        if (m_stopFlag) return "";
        m_hasInput = false;
        return m_pendingInput.toStdString();
    };

    std::string sourceUtf8 = m_sourceCode.toUtf8().data();
    std::string fileUtf8 = m_filePath.toUtf8().data();

    // 1. Lexer
    Doruk::Lexer lexer(sourceUtf8, fileUtf8);
    auto lexerSonuc = lexer.tara();

    if (!lexerSonuc.basarili()) {
        for (const auto& err : lexerSonuc.tanilar.tanilar()) {
            emit errorReceived(err.satir, err.sutun, QString::fromStdString(err.mesaj));
        }
        emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), false, static_cast<int>(lexerSonuc.tanilar.tanilar().size()));
        return;
    }

    if (m_stopFlag) return;

    // 2. Parser
    Doruk::Parser parser(lexerSonuc.tokenler, fileUtf8);
    auto parserSonuc = parser.ayristir();

    if (!parserSonuc.basarili() || !parserSonuc.program) {
        for (const auto& err : parserSonuc.tanilar.tanilar()) {
            emit errorReceived(err.satir, err.sutun, QString::fromStdString(err.mesaj));
        }
        emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), false, static_cast<int>(parserSonuc.tanilar.tanilar().size()));
        return;
    }

    if (m_stopFlag) return;

    // 3. Semantic Analyzer
    Doruk::SemanticAnalyzer analyzer(fileUtf8);
    auto anlamSonuc = analyzer.analiz(*parserSonuc.program);

    if (!anlamSonuc.basarili()) {
        for (const auto& err : anlamSonuc.tanilar.tanilar()) {
            emit errorReceived(err.satir, err.sutun, QString::fromStdString(err.mesaj));
        }
        emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), false, static_cast<int>(anlamSonuc.tanilar.tanilar().size()));
        return;
    }

    if (m_stopFlag) return;

    // 4. Interpreter
    Doruk::Interpreter interpreter(ciktiCallback, girisCallback);
    try {
        auto yorumSonuc = interpreter.calistir(*parserSonuc.program);

        if (!yorumSonuc.basarili) {
            for (const auto& err : yorumSonuc.tanilar.tanilar()) {
                emit errorReceived(err.satir, err.sutun, QString::fromStdString(err.mesaj));
            }
            emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), false, static_cast<int>(yorumSonuc.tanilar.tanilar().size()));
            return;
        }

    } catch (const std::exception& e) {
        emit errorReceived(0, 0, QString("Çalışma Hatası: %1").arg(e.what()));
        emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), false, 1);
        return;
    }

    emit executionFinished(QString::number(timer.elapsed() / 1000.0, 'f', 3), true, 0);
}

