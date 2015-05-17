#include <QtWidgets>

#include "qsscodeeditor.h"

QssCodeEditor::QssCodeEditor(QWidget *parent) :
    QPlainTextEdit(parent), comp(0)
{
    lineNumberArea = new LineNumberArea(this);
    QFont font;//("Courier New", 10);
    font.setFamily(QStringLiteral("Courier New"));
    font.setPointSize(10);
    font.setKerning(false);
    font.setStyleStrategy(QFont::NoAntialias);
    setFont(font);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}
//-----------------------------------------------------------------------
int QssCodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space;
}
//-----------------------------------------------------------------------
///////////Code Completer//////////////////
void QssCodeEditor::setCompleter(QCompleter *completer)
{
    if (comp)
        QObject::disconnect(comp, 0, this, 0);

    comp = completer;

    if (!comp)
        return;

    comp->setWidget(this);
    comp->setCompletionMode(QCompleter::PopupCompletion);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(comp, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}
//-----------------------------------------------------------------------
QCompleter *QssCodeEditor::completer() const
{
    return comp;
}
//-----------------------------------------------------------------------
void QssCodeEditor::insertCompletion(const QString& completion)
{
    if (comp->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - comp->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}
//-----------------------------------------------------------------------
QString QssCodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
//-----------------------------------------------------------------------
void QssCodeEditor::focusInEvent(QFocusEvent *e)
{
    if (comp)
        comp->setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}
//-----------------------------------------------------------------------
void QssCodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (comp && comp->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!comp || !isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!comp || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 3
                      || eow.contains(e->text().right(1)))) {
        comp->popup()->hide();
        return;
    }

    if (completionPrefix != comp->completionPrefix()) {
        comp->setCompletionPrefix(completionPrefix);
        comp->popup()->setCurrentIndex(comp->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(comp->popup()->sizeHintForColumn(0)
                + comp->popup()->verticalScrollBar()->sizeHint().width());
    comp->complete(cr); // popup it up!
}
//-----------------------------------------------------------------------
void QssCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth()+2, -6, 0, 0);
}                                          //↑↑↑↑
//-----------------------------------------------------------------------
void QssCodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}
//-----------------------------------------------------------------------
void QssCodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}
//-----------------------------------------------------------------------
void QssCodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::green).lighter(190);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}
//-----------------------------------------------------------------------
void QssCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height()+5,
                             Qt::AlignRight, number);      // ↑^↑-stupid, therefore→↑^↑
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
//-----------------------------------------------------------------------
