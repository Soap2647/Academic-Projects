#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class DorukHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit DorukHighlighter(QTextDocument *parent = nullptr);

    // Theme Update Hooks
    void setTheme(const QColor& keyword, const QColor& stringColor, const QColor& number, const QColor& comment, const QColor& operatorColor);
    void updateFormats();

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat operatorFormat;

    QColor m_keywordColor = Qt::darkBlue;
    QColor m_stringColor = Qt::darkGreen;
    QColor m_numberColor = Qt::darkRed;
    QColor m_commentColor = Qt::gray;
    QColor m_operatorColor = QColor(180, 180, 180);
};
