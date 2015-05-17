//   ColorWidget by SparQtech (Oleg Belyavsky) 2014
//-----------------------------------------------------------------------
#include <QHBoxLayout>
#include <QColor>
//#include <QColorDialog>
#include "qcolordialog.h" //↓↓↓↓↓↓↓↓↓
#include <QPushButton>
#include <QStyle>
#include <QMessageBox>
#include <QMenu>
#include <QContextMenuEvent>
#include "colorwidget.h"
#include "mainwindow.h"

//-----------------------------------------------------------------------
ColorWidget::ColorWidget(QWidget *parent) : QWidget(parent)
{
    colorLineEdit = new QLineEdit(this);
    colorLineEdit->setInputMask("\\#HHHHHH"); //("\\#HHHHHHhh");
    colorLineEdit->setStyleSheet("font:courier new;");
    colorLineEdit->setMinimumHeight(16);
    colorLineEdit->setMaximumHeight(16);
    colorLineEdit->setFrame(false);

    colorLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    clearAct = new QAction(tr("Cl&ear"), colorLineEdit);

    connect(clearAct, SIGNAL(triggered()), this, SLOT(Clear()));
    connect(colorLineEdit, SIGNAL(textChanged(QString)), this, SLOT(colorChanged(QString)));

    colorButton = new QPushButton(this);
    colorButton->setMinimumSize(16, 16);
    colorButton->setMaximumSize(16, 16);
    colorButton->setIcon(QIcon(":/res/color.png"));
    colorButton->setToolTip(tr("Color"));
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColorTxt()));
    connect(this, SIGNAL(setTxtColor(QString)), colorLineEdit, SLOT(setText(QString)));

    gradientButton = new QPushButton(this);
    gradientButton->setMinimumSize(16, 16);
    gradientButton->setMaximumSize(16, 16);
    gradientButton->setIcon(QIcon(":/res/gradient.png"));
    gradientButton->setToolTip(tr("Gradient"));
    connect(gradientButton, SIGNAL(clicked()), this, SLOT(getGradient()));

    widgetGradient = new QFrame(this);
    widgetGradient->setGeometry(1, 1, 72, 16);
    this->setAccessibleName("color");

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(colorLineEdit);
    layout->addWidget(widgetGradient);
    layout->addWidget(colorButton);
    layout->addWidget(gradientButton);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    setWindowTitle("color();");
}
//-----------------------------------------------------------------------
void ColorWidget::setColorTxt()
{
    QColor colorOld, colorNew;
    QString sO = colorLineEdit->text();

    if (sO == "#") sO = "#ffffff";
    colorOld.setNamedColor(sO);
    colorNew = QColorDialog::getColor(colorOld, this);
                                     //↑↑↑↑↑↑↑ - Don't work at Qt 5.4 :(
                                     //UPD: QTBUG-42988.patch  05/Jan/15 9:24 AM 2 kB
    if (colorNew.isValid()){
        QString sF = colorNew.name();
        colorLineEdit->show();
        widgetGradient->hide();
        emit setTxtColor(sF);
    }
}
//-----------------------------------------------------------------------
void ColorWidget::colorChanged(const QString &s)
{
    QColor color;
    color.setNamedColor(s);
    QString sV, sF = color.name();
    if (color.isValid()){
        int i = color.red() + color.green() + color.blue()/2;
        if (i <= 400) sV = "white";
        else sV = "black";
        colorLineEdit->setStyleSheet("color:"+sV+";background-color:" + sF);
        setStatusTip(sF);
        setWindowTitle("color(" + sF + ");");
        emit atas();
     }
    else setStatusTip("");
}
//-----------------------------------------------------------------------
void ColorWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(clearAct);
    menu->exec(event->globalPos());
    delete menu;
}
//-----------------------------------------------------------------------
void ColorWidget::Clear()
{
    colorLineEdit->clear();
    colorLineEdit->setStyleSheet("color:grey;background-color:white");
    widgetGradient->setStyleSheet("background:");
    setStatusTip("");
    setWindowTitle("color();");
    emit atas();
}
//-----------------------------------------------------------------------
void ColorWidget::getGradient()
{
    gradientDialog = new GradientDialog(this);
    connect(gradientDialog, SIGNAL(sendOut(QString)), this, SLOT(getGradientCode(QString)));
    gradientDialog->setIn(widgetGradient->styleSheet());
    gradientDialog->createOutputCode();
    if (gradientDialog->exec() == QDialog::Accepted){
        colorLineEdit->hide();
        widgetGradient->show();
        this->setWindowTitle(this->statusTip()+";");
        emit atas();
    }
    delete gradientDialog;
}
//-----------------------------------------------------------------------
void ColorWidget::getGradientCode(const QString &s)
{
    setStatusTip(s);
    widgetGradient->setStyleSheet("background: " + s);
}
//-----------------------------------------------------------------------

