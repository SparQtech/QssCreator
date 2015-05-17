#include <QLayout>
#include <QEvent>
#include <QContextMenuEvent>
#include "spinwidgets.h"

//-----------------------------------------------------------------------
SSpinBox::SSpinBox(QWidget *parent) :
    QSpinBox(parent)
{
    spinBox = new QSpinBox(this);
    spinBox->setSpecialValueText(tr("off"));
    spinBox->setAlignment(Qt::AlignRight);
    spinBox->setContextMenuPolicy(Qt::NoContextMenu); //(!)
    spinBox->setButtonSymbols(QAbstractSpinBox::PlusMinus);

    offAct = new QAction(tr("Set off"), spinBox);
    connect(offAct, SIGNAL(triggered()), this, SLOT(setOff()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(spinBox);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    connect(spinBox, SIGNAL(valueChanged(QString)), this, SLOT(setStatustip(QString)));
    connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(valueChange(int)));
    this->setAccessibleName("int");
    this->setWindowTitle("int" + QString("(%1, %2)").arg(spinBox->minimum()).arg(spinBox->maximum()) +
                         this->statusTip() + ";");
}
//-----------------------------------------------------------------------
SSpinBox::~SSpinBox()
{

}
//-----------------------------------------------------------------------
void SSpinBox::setValues(int min, int max, int value, const QString suffix, bool off){
    spinBox->setRange(min, max);
    spinBox->setValue(value);
    spinBox->setSuffix(suffix);
    QString sval;
    sval.setNum(value);
    setStatustip(sval + suffix);
    if (off){
        setOff();
        sval = "off";
    }
    if (suffix != "") sval = "[" + suffix + "]";
    this->setWindowTitle("int" + QString("(%1, %2)").arg(min).arg(max)
                         + sval + ";");
}

//-----------------------------------------------------------------------
void SSpinBox::setStatustip(QString s)
{
     setStatusTip(s + this->suffix());
}
//-----------------------------------------------------------------------
void SSpinBox::valueChange(int val)
{
    QString suff = spinBox->suffix(), sval;
    if (suff != "") suff = "[" + suff + "];";
    if (val == spinBox->minimum()+1) spinBox->setValue(0); //hack
    if (val == spinBox->minimum())
        sval = "off";
    else
        sval.setNum(spinBox->value());
    this->setWindowTitle(QString("int(%1, %2)").arg(spinBox->minimum()).arg(spinBox->maximum())
                         + sval + suff);
    emit atas();
}
//-----------------------------------------------------------------------
void SSpinBox::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    menu->addAction(offAct);
    menu->exec(event->globalPos());
    delete menu;
}
//-----------------------------------------------------------------------
void SSpinBox::setOff(){
    int i = spinBox->minimum();
    spinBox->setValue(i);
}
//-----------------------------------------------------------------------
