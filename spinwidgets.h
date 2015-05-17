#ifndef SPINWIDGETS_H
#define SPINWIDGETS_H

#include <QWidget>
#include <QSpinBox>
#include <QAction>
#include <QMenu>

class SSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SSpinBox(QWidget *parent = 0);
    ~SSpinBox();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

signals:
    void atas();

public slots:
    void setStatustip(QString s);
    void setValues(int min, int max, int value, const QString suffix, bool off);
    void valueChange(int val);
    void setOff();

private:
    QSpinBox *spinBox;
    QAction *offAct;

};

#endif // SPINWIDGETS_H
