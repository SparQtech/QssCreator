#include <QColor>
#include <QColorDialog>
#include <QMessageBox>
#include "gradientdialog.h"
#include "ui_gradientdialog.h"

GradientDialog::GradientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GradientDialog)
{
    ui->setupUi(this);

}
//-----------------------------------------------------------------------
GradientDialog::~GradientDialog()
{
    delete ui;
}
//-----------------------------------------------------------------------
void GradientDialog::createOutputCode()
{
    switch (ui->comboGrad->currentIndex()) {
    case 0: createLinear();
        break;
    case 1: createRadial();
        break;
    case 2: createConical();
        break;
    default:
            ;
    }
}
//-----------------------------------------------------------------------
QString GradientDialog::colorValidate()
{
    QString sColor;
    if (QColor::isValidColor(ui->colorEdit0->text()))
        sColor = ui->dSpinStop0->text() + " " + ui->colorEdit0->text();
    if (QColor::isValidColor(ui->colorEdit1->text()))
        sColor += ", " + ui->dSpinStop1->text() + " " + ui->colorEdit1->text();
    if (QColor::isValidColor(ui->colorEdit2->text()))
        sColor += ", " + ui->dSpinStop2->text() + " " + ui->colorEdit2->text();
    if (QColor::isValidColor(ui->colorEdit3->text()))
        sColor +=  ", " + ui->dSpinStop3->text() + " " + ui->colorEdit3->text();
    return sColor;
}
//-----------------------------------------------------------------------
void GradientDialog::createLinear()
{
    QString sOut;
    sOut = ui->comboGrad->currentText() + "(" + ui->dSpinX1->text() + ", " +
        ui->dSpinY1->text() + ", " + ui->dSpinX2->text() + ", " + ui->dSpinY2->text() +
        ", " + colorValidate() + ")";
    ui->gradientTextEdit->document()->setPlainText(sOut);
    ui->frame->setStyleSheet("background: " + sOut);
}
//-----------------------------------------------------------------------
void GradientDialog::createRadial()
{
    QString sOut;
    sOut = ui->comboGrad->currentText() + "(" + ui->dSpinX1->text() + ", " +
        ui->dSpinY1->text() + ", " + ui->radiusSpinBox->text() + ", " + ui->dSpinX2->text()
        + ", " + ui->dSpinY2->text() + ", " + colorValidate() + ")";
    ui->gradientTextEdit->document()->setPlainText(sOut);
    ui->frame->setStyleSheet("background: " + sOut);
}
//-----------------------------------------------------------------------
void GradientDialog::createConical()
{
    QString sOut;
    sOut = ui->comboGrad->currentText() + "(" + ui->dSpinX1->text() + ", " +
        ui->dSpinY1->text() + ", " + ui->angleSpinBox->text() + ", " + colorValidate() + ")";
    ui->gradientTextEdit->document()->setPlainText(sOut);
    ui->frame->setStyleSheet("background: " + sOut);
}
//-----------------------------------------------------------------------
void GradientDialog::changeGradType(int type)
{
    switch (type) {
    case 0: ui->dSpinX1->setPrefix("x1:");  //Linear
            ui->dSpinY1->setPrefix("y1:");
            ui->dSpinX2->setPrefix("x2:");
            ui->dSpinY2->setPrefix("y2:");
            ui->radiusSpinBox->setEnabled(false);
            ui->angleSpinBox->setEnabled(false);
            ui->dSpinX2->setEnabled(true);
            ui->dSpinY2->setEnabled(true);
            createLinear();
        break;
    case 1: ui->dSpinX1->setPrefix("cx:");  //Radial
            ui->dSpinY1->setPrefix("cy:");
            ui->dSpinX2->setPrefix("fx:");
            ui->dSpinY2->setPrefix("fy:");
            ui->radiusSpinBox->setEnabled(true);
            ui->angleSpinBox->setEnabled(false);
            ui->dSpinX2->setEnabled(true);
            ui->dSpinY2->setEnabled(true);
            createRadial();
        break;
    case 2: ui->dSpinX1->setPrefix("cx:");  //Conical
            ui->dSpinY1->setPrefix("cy:");
            ui->dSpinX2->setEnabled(false);
            ui->dSpinY2->setEnabled(false);
            ui->radiusSpinBox->setEnabled(false);
            ui->angleSpinBox->setEnabled(true);
            createConical();
        break;

    default:
            ;
    }
}
//-----------------------------------------------------------------------
void GradientDialog::setColor0()
{
    QColor colorOld, colorNew;
    QString sO = ui->colorEdit0->text();
    if (sO == "#") sO = "#ffffff";
    colorOld.setNamedColor(sO); //
    colorNew = QColorDialog::getColor(colorOld, this);
    ui->colorEdit0->setText(colorNew.name());
    ui->btnColor0->setStyleSheet("background-color: " + ui->colorEdit0->text());
}
//-----------------------------------------------------------------------
void GradientDialog::setColor1()
{
    QColor colorOld, colorNew;
    QString sO = ui->colorEdit1->text();
    if (sO == "#") sO = "#ffffff";
    colorOld.setNamedColor(sO); //
    colorNew = QColorDialog::getColor(colorOld, this);
    ui->colorEdit1->setText(colorNew.name());
    ui->btnColor1->setStyleSheet("background-color: " + ui->colorEdit1->text());
}
//-----------------------------------------------------------------------
void GradientDialog::setColor2()
{
    QColor colorOld, colorNew;
    QString sO = ui->colorEdit2->text();
    if (sO == "#") sO = "#ffffff";
    colorOld.setNamedColor(sO); //
    colorNew = QColorDialog::getColor(colorOld, this);
    ui->colorEdit2->setText(colorNew.name());
    ui->btnColor2->setStyleSheet("background-color: " + ui->colorEdit2->text());
}
//-----------------------------------------------------------------------
void GradientDialog::setColor3()
{
    QColor colorOld, colorNew;
    QString sO = ui->colorEdit3->text();
    if (sO == "#") sO = "#ffffff";
    colorOld.setNamedColor(sO); //
    colorNew = QColorDialog::getColor(colorOld, this);
    ui->colorEdit3->setText(colorNew.name());
    ui->btnColor3->setStyleSheet("background-color: " + ui->colorEdit3->text());
}
//-----------------------------------------------------------------------
void GradientDialog::ResetXY()
{
    switch (ui->comboGrad->currentIndex()) {
    case 0:   ui->dSpinX1->setValue(0.5);
              ui->dSpinY1->setValue(0.0);
              ui->dSpinX2->setValue(0.5);
              ui->dSpinY2->setValue(1.0);
        break;
    case 1:   ui->dSpinX1->setValue(0.5);
              ui->dSpinY1->setValue(0.5);
              ui->dSpinX2->setValue(0.5);
              ui->dSpinY2->setValue(0.5);
              ui->radiusSpinBox->setValue(0.5);
        break;
    case 2:   ui->dSpinX1->setValue(0.5);
              ui->dSpinY1->setValue(0.5);
              ui->angleSpinBox->setValue(90);
        break;
    default:
            ;
    }
}
//----------------------------------------------------------------------
void GradientDialog::setColorBtn0()
{
    if (QColor::isValidColor(ui->colorEdit0->text()))
        ui->btnColor0->setStyleSheet("background-color: " + ui->colorEdit0->text());
    else ui->btnColor0->setStyleSheet("border: 2px solid red");
}
//----------------------------------------------------------------------
void GradientDialog::setColorBtn1()
{
    if (QColor::isValidColor(ui->colorEdit1->text()))
        ui->btnColor1->setStyleSheet("background-color: " + ui->colorEdit1->text());
    else ui->btnColor1->setStyleSheet("border: 2px solid red");
}
//----------------------------------------------------------------------
void GradientDialog::setColorBtn2()
{
    if (QColor::isValidColor(ui->colorEdit2->text()))
        ui->btnColor2->setStyleSheet("background-color: " + ui->colorEdit2->text());
    else ui->btnColor2->setStyleSheet("border: 2px solid red");
}
//----------------------------------------------------------------------
void GradientDialog::setColorBtn3()
{
    if (QColor::isValidColor(ui->colorEdit3->text()))
        ui->btnColor3->setStyleSheet("background-color: " + ui->colorEdit3->text());
    else ui->btnColor3->setStyleSheet("border: 2px solid red");
}
//-----------------------------------------------------------------------
void GradientDialog::setOut()
{
    sOutCode = ui->gradientTextEdit->toPlainText();
    emit sendOut(sOutCode);
}
//-----------------------------------------------------------------------
void GradientDialog::setIn(const QString &sInCode)
{
    QString str = sInCode;
    ui->frame->setStyleSheet(str);
    str = str.remove("background:");
    str = str.remove(";");
    str = str.trimmed();
    ui->gradientTextEdit->document()->setPlainText(str);

    if (str.startsWith("qlineargradient")){
        ui->comboGrad->setCurrentIndex(0);
        str = str.remove("qlineargradient");
    }
    else if (str.startsWith("qradialgradient")){
        ui->comboGrad->setCurrentIndex(1);
        str = str.remove("qradialgradient");
    }
    else if (str.startsWith("qconicalgradient")){
        ui->comboGrad->setCurrentIndex(2);
        str = str.remove("qconicalgradient");
    }
    str = str.remove("(");
    str = str.remove(")");
    str = str.trimmed();
    QStringList list = str.split(",");
    QString stri;
    int countStop = 0;
    for (int i = 0; i < list.size(); ++i){
        stri = list.at(i).trimmed();
        if (stri.contains("x1") || stri.contains("cx"))
            ui->dSpinX1->setValue(StoD(stri));
        else if (stri.contains("x2") || stri.contains("fx"))
            ui->dSpinX2->setValue(StoD(stri));
        else if (stri.contains("y1") || stri.contains("cy"))
            ui->dSpinY1->setValue(StoD(stri));
        else if (stri.contains("y2") || stri.contains("fy"))
            ui->dSpinY2->setValue(StoD(stri));
        else if (stri.contains("radius"))
            ui->radiusSpinBox->setValue(StoD(stri));
        else if (stri.contains("angle")){
            QString sangle = stri;
            int j = stri.length() - sangle.indexOf(":") -1;
            sangle = sangle.right(j).trimmed();
            ui->angleSpinBox->setValue(sangle.toInt());
        }
        else if (stri.contains("stop")){
            stri = stri.remove(0, stri.indexOf(":")+1);
            stri = stri.trimmed();
            QStringList lst = stri.split(" ");
            QString color = lst.at(1).trimmed();
            double stop = lst.at(0).trimmed().toDouble();
            if (countStop <= 3){
                switch (countStop) {
                case 0: ui->dSpinStop0->setValue(stop);
                        ui->colorEdit0->setText(color);
                    break;
                case 1: ui->dSpinStop1->setValue(stop);
                        ui->colorEdit1->setText(color);
                    break;
                case 2: ui->dSpinStop2->setValue(stop);
                        ui->colorEdit2->setText(color);
                    break;
                case 3: ui->dSpinStop3->setValue(stop);
                        ui->colorEdit3->setText(color);
                    break;
                default:
                    ;
                }
                ++countStop;
            }
        }
    }
}
//-----------------------------------------------------------------------
double GradientDialog::StoD(const QString &s)
{
    QString str = s;
    int i = str.length() - str.indexOf(":") - 1;
    str = str.right(i).trimmed();
    return str.toDouble();
}
//-----------------------------------------------------------------------

