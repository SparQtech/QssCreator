#include "qsshighlighter.h"

QssHighlighter::QssHighlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
//    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "alternate-background-color" << "background" << "background-color"
                    << "background-image" << "background-repeat" << "background-position"
                    << "background-attachment" << "background-clip" << "background-origin"
                    << "border" << "border-top" << "border-right" << "border-bottom"
                    << "border-left" << "border-color" << "border-top-color" << "border-right-color"
                    << "border-bottom-color" << "border-left-color" << "border-image"
                    << "border-radius" << "border-top-left-radius" << "border-top-right-radius"
                    << "border-bottom-right-radius" << "border-bottom-left-radius"
                    << "border-style" << "border-top-style" << "border-right-style"
                    << "border-bottom-style" << "border-left-style" << "border-width"
                    << "border-top-width" << "border-right-width" << "border-bottom-width"
                    << "border-left-width" << "bottom" << "button-layout" << "color"
                    << "dialogbuttonbox-buttons-have-icons" << "font" << "font-family"
                    << "font-size" << "font-style" << "font-weight" << "gridline-color"
                    << "height" << "icon-size" << "image" << "image-position"
                    << "left" << "lineedit-password-character" << "margin" << "margin-top"
                    << "margin-right" << "margin-bottom" << "margin-left" << "max-height"
                    << "max-width" << "messagebox-text-interaction-flags" << "min-height"
                    << "min-width" << "opacity" << "padding" << "padding-top"
                    << "padding-right" << "padding-bottom" << "padding-left"
                    << "paint-alternating-row-colors-for-empty-area" << "position" << "right"
                    << "selection-background-color" << "selection-color" << "show-decoration-selected"
                    << "spacing" << "subcontrol-origin" << "subcontrol-position" << "text-align"
                    << "text-decoration" << "top" << "width";
    foreach (const QString &pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
//-----------------------------------------------------------------------
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    pseudoStateFormat.setForeground(Qt::red);
    rule.pattern = QRegExp(":[^{) \n]*");
    rule.format = pseudoStateFormat;
    highlightingRules.append(rule);

    urlResourceFormat.setForeground(Qt::green);
    rule.pattern = QRegExp(":/[^)\n]*");
    rule.format = urlResourceFormat;
    highlightingRules.append(rule);

    subControlFormat.setForeground(Qt::darkYellow);
    rule.pattern = QRegExp("::[^ :\n]*");
    rule.format = subControlFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGreen);
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}
//-----------------------------------------------------------------------
void QssHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {
        int endIndex = commentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
//-----------------------------------------------------------------------
