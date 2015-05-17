#ifndef QSSCODEEDITOR_H
#define QSSCODEEDITOR_H

#include <QPlainTextEdit>
#include <QObject>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QCompleter;
QT_END_NAMESPACE

class LineNumberArea;

//-----------------------------------------------------------------------
class QssCodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit QssCodeEditor(QWidget *parent = 0);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

    void setCompleter(QCompleter *comp);
    QCompleter *completer() const;

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &, int);
    void insertCompletion(const QString &completion);

private:
    QWidget *lineNumberArea;
    QString textUnderCursor() const;
    QCompleter *comp;
};
//-----------------------------------------------------------------------
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(QssCodeEditor *editor) : QWidget(editor) {
        codeEditor = editor;
    }

    QSize sizeHint() const {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    QssCodeEditor *codeEditor;

};

#endif // QSSCODEEDITOR_H
