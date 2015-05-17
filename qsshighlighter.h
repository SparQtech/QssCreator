#ifndef QSSHIGHLIGHTER_H
#define QSSHIGHLIGHTER_H

#include <QSyntaxHighlighter>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

class QssHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit QssHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat subControlFormat;
    QTextCharFormat pseudoStateFormat;
    QTextCharFormat urlResourceFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;

};

#endif // QSSHIGHLIGHTER_H
