#include "DorukHighlighter.h"

DorukHighlighter::DorukHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
    commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
    
    updateFormats();
}

void DorukHighlighter::setTheme(const QColor& keyword, const QColor& stringColor, const QColor& number, const QColor& comment, const QColor& operatorColor) {
    m_keywordColor = keyword;
    m_stringColor = stringColor;
    m_numberColor = number;
    m_commentColor = comment;
    m_operatorColor = operatorColor;
    updateFormats();
}

void DorukHighlighter::updateFormats() {
    highlightingRules.clear();
    HighlightingRule rule;

    // ── Operators ──────────────────────────────────────────
    operatorFormat.setForeground(m_operatorColor);
    const QString operatorPatterns[] = {
        QStringLiteral("\\+"), QStringLiteral("-"), QStringLiteral("\\*"), QStringLiteral("/"),
        QStringLiteral("=="), QStringLiteral("!="), QStringLiteral(">="), QStringLiteral("<="),
        QStringLiteral(">"), QStringLiteral("<"), QStringLiteral("="), QStringLiteral("&&"),
        QStringLiteral("\\|\\|"), QStringLiteral("!")
    };
    for (const QString &pattern : operatorPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = operatorFormat;
        highlightingRules.append(rule);
    }

    // ── Keywords ────────────────────────────────────────────
    keywordFormat.setForeground(m_keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bdeğişken\\b"), QStringLiteral("\\beğer\\b"), QStringLiteral("\\bdeğilse\\b"),
        QStringLiteral("\\bdöngü\\b"), QStringLiteral("\\biçin\\b"), QStringLiteral("\\bfonksiyon\\b"),
        QStringLiteral("\\bdöndür\\b"), QStringLiteral("\\byazdır\\b"), QStringLiteral("\\boku\\b"),
        QStringLiteral("\\bdoğru\\b"), QStringLiteral("\\byanlış\\b"), QStringLiteral("\\bboş\\b"),
        QStringLiteral("\\bve\\b"), QStringLiteral("\\bveya\\b"), QStringLiteral("\\bdeğil\\b"),
        QStringLiteral("\\bkır\\b"), QStringLiteral("\\bdevam\\b"), QStringLiteral("\\bsınıf\\b"),
        QStringLiteral("\\byeni\\b"), QStringLiteral("\\bbu\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // ── Numbers ─────────────────────────────────────────────
    numberFormat.setForeground(m_numberColor);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[0-9]+(\\.[0-9]+)?\\b"));
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // ── Strings ─────────────────────────────────────────────
    quotationFormat.setForeground(m_stringColor);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // ── Single Line Comments ────────────────────────────────
    singleLineCommentFormat.setForeground(m_commentColor);
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // ── Multi Line Comments ─────────────────────────────────
    multiLineCommentFormat.setForeground(m_commentColor);
    multiLineCommentFormat.setFontItalic(true);

    rehighlight();
}

void DorukHighlighter::highlightBlock(const QString &text)
{
    // Apply basic rules
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Apply multiline comment
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

