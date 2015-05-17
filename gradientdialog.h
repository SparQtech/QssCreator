#ifndef GRADIENTDIALOG_H
#define GRADIENTDIALOG_H

#include <QDialog>

namespace Ui {
class GradientDialog;
}

class GradientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GradientDialog(QWidget *parent = 0);
    ~GradientDialog();
    QString sOutCode;
    void setIn(const QString &sInCode);

protected:

signals:
    void sendOut(QString);

public slots:
    void createOutputCode();

private slots:
    void createLinear();
    void createRadial();
    void createConical();
    void changeGradType(int type);
    void setColor0();
    void setColor1();
    void setColor2();
    void setColor3();
    void ResetXY();
    void setColorBtn0();
    void setColorBtn1();
    void setColorBtn2();
    void setColorBtn3();
    void setOut();

private:
    Ui::GradientDialog *ui;
    QString colorValidate();
    double StoD(const QString &s);
};

#endif // GRADIENTDIALOG_H
