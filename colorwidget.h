//   ColorWidget by SparQtech (Oleg Belyavsky)
//-----------------------------------------------------------------------
#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QWidget>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QAction>
#include "gradientdialog.h"

class ColorWidget : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("author", "SparQtech")
public:
    explicit ColorWidget(QWidget *parent = 0);
    QLineEdit *colorLineEdit;
    QWidget *widgetGradient;

protected:
    void contextMenuEvent(QContextMenuEvent *event);

public slots:
    void getGradientCode(const QString &s);
    void colorChanged(const QString &s);

signals:
    void setTxtColor(const QString &s);
    void atas();

private slots:
     void setColorTxt();
    void getGradient();
    void Clear();


private:
    QPushButton *colorButton;
    QPushButton *gradientButton;
    QAction *clearAct;
    GradientDialog *gradientDialog;
    QString sGradientOutCode;
};

#endif // COLORWIDGET_H
