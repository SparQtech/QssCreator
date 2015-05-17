#ifndef COMBOWIDGETS_H
#define COMBOWIDGETS_H

#include <QWidget>
#include <QComboBox>
#include <QFontComboBox>

//-----------------------------------------------------------------------
class BorderStyleComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit BorderStyleComboWidget(QWidget *parent = 0);
    bool setValue(const QString string);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboBorderStyle;
    QStringList bordersList;

};
//-----------------------------------------------------------------------
class RepeatComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit RepeatComboWidget(QWidget *parent = 0);
    bool setValue(const QString string);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboRepeat;

};
//-----------------------------------------------------------------------
class AttachmentComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit AttachmentComboWidget(QWidget *parent = 0);
    bool setValue(const QString string);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboAttachment;

};
//-----------------------------------------------------------------------
class AlignmentComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit AlignmentComboWidget(QWidget *parent = 0);
    void setValues(QString values);

signals:
    void atas();

public slots:
    void changeValues(int index);
    void setStatustip(QString s);
    void SetStatustip(QString s);

private:
    QComboBox *comboAlignment1;
    QComboBox *comboAlignment2;
};
//-----------------------------------------------------------------------
class OriginComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit OriginComboWidget(QWidget *parent = 0);
    bool setValue(const QString string);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboOrigin;

};
//-----------------------------------------------------------------------
class FontComboWidget : public QFontComboBox
{
    Q_OBJECT
public:
    explicit FontComboWidget(QWidget *parent = 0);
    void setValue(QString value);

signals:
    void atas();

public slots:
    void setStatustip(QString s);


private:
    QFontComboBox *comboFont;
};
//-----------------------------------------------------------------------
class FontStyleComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit FontStyleComboWidget(QWidget *parent = 0);
    void setValue(QString values);

signals:
    void atas();

public slots:
    void setStatustip(QString s);


private:
    QComboBox *comboFontStyle;
};
//-----------------------------------------------------------------------
class FontWeightComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit FontWeightComboWidget(QWidget *parent = 0);
    void setValue(QString values);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboFontWeight;
};
//-----------------------------------------------------------------------
class TextDecorComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit TextDecorComboWidget(QWidget *parent = 0);
    void setValue(QString values);

signals:
    void atas();

public slots:
    void setStatustip(QString s);


private:
    QComboBox *comboTextDecor;
};
//-----------------------------------------------------------------------
class MultiComboWidget : public QComboBox
{
    Q_OBJECT
public:
    explicit MultiComboWidget(QWidget *parent = 0);
    void setValues(QString string);

signals:
    void atas();

public slots:
    void setStatustip(QString s);

private:
    QComboBox *comboMulti;
};
//-----------------------------------------------------------------------
#endif // COMBOWIDGETS_H
