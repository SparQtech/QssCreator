#include <QBoxLayout>
#include "combowidgets.h"
//-----------------------------------------------------------------------
BorderStyleComboWidget::BorderStyleComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboBorderStyle = new QComboBox(this);
    comboBorderStyle->setFrame(false);

    bordersList << "" << "none" << "dotted" << "dashed" << "solid" << "double" << "dot-dash"
                << "dot-dot-dash" << "groove" << "ridge" << "inset" << "outset";
    comboBorderStyle->addItems(bordersList);
    comboBorderStyle->setIconSize(QSize(32, 16));
    comboBorderStyle->setItemIcon(2, QIcon(":res/dotted.png"));
    comboBorderStyle->setItemIcon(3, QIcon(":res/dashed.png"));
    comboBorderStyle->setItemIcon(4, QIcon(":res/solid.png"));
    comboBorderStyle->setItemIcon(5, QIcon(":res/double.png"));
    comboBorderStyle->setItemIcon(6, QIcon(":res/dot-dash.png"));
    comboBorderStyle->setItemIcon(7, QIcon(":res/dot-dot-dash.png"));
    comboBorderStyle->setItemIcon(8, QIcon(":res/groove.png"));
    comboBorderStyle->setItemIcon(9, QIcon(":res/ridge.png"));
    comboBorderStyle->setItemIcon(10, QIcon(":res/inset.png"));
    comboBorderStyle->setItemIcon(11, QIcon(":res/outset.png"));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(comboBorderStyle);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    connect(comboBorderStyle, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
    this->setAccessibleName("borderstyle");
    this->setWindowTitle("borderstyle();");
}
//-----------------------------------------------------------------------
bool BorderStyleComboWidget::setValue(const QString string)
{
   for (int i = 0; i < bordersList.size(); ++i){
        if (bordersList.at(i) == string){
            comboBorderStyle->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------
void BorderStyleComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    this->setWindowTitle("borderstyle(" + s + ");");
    emit atas();
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
RepeatComboWidget::RepeatComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboRepeat = new QComboBox(this);
    comboRepeat->setFrame(false);
    QStringList repeats;
        repeats << "" << "repeat-x" << "repeat-y" << "repeat-xy" << "no-repeat";
    comboRepeat->addItems(repeats);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(comboRepeat);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    connect(comboRepeat, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
    this->setAccessibleName("repeat");
    this->setWindowTitle("repeat();");
}
//-----------------------------------------------------------------------
bool RepeatComboWidget::setValue(const QString string)
{
    comboRepeat->setCurrentText(string);
    return true;
}
//-----------------------------------------------------------------------
void RepeatComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    this->setWindowTitle("repeat(" + s + ");");
    emit atas();
    switch (comboRepeat->currentIndex()) {
    case 0: setToolTip("");
        break;
    case 1: setToolTip(tr("Repeat horizontally"));
        break;
    case 2: setToolTip(tr("Repeat vertically"));
        break;
    case 3: setToolTip(tr("Repeat horizontally and vertically"));
        break;
    case 4: setToolTip(tr("Don't repeat"));
        break;
    default: setToolTip("");
            ;
    }
    emit atas();
}
//-----------------------------------------------------------------------
//OriginComboWidget
//-----------------------------------------------------------------------
OriginComboWidget::OriginComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboOrigin = new QComboBox(this);
    comboOrigin->setFrame(false);
    QStringList attachment;
        attachment << "" << "margin" << "border" << "padding" << "content";
    comboOrigin->addItems(attachment);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboOrigin);
    setLayout(layout);
    connect(comboOrigin, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
    setAccessibleName("attachment");
    setWindowTitle("attachment();");
}
//-----------------------------------------------------------------------
bool OriginComboWidget::setValue(const QString string)
{
    if (string == "" || string == "scroll" || string == "fixed"){
        comboOrigin->setCurrentText(string);
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------
void OriginComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("attachment(" + s + ");");
    emit atas();
}
//-----------------------------------------------------------------------
AttachmentComboWidget::AttachmentComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboAttachment = new QComboBox(this);
    comboAttachment->setFrame(false);
    QStringList attachment;
        attachment << "" << "scroll" << "fixed";
    comboAttachment->addItems(attachment);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboAttachment);
    setLayout(layout);
    connect(comboAttachment, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
    setAccessibleName("attachment");
    setWindowTitle("attachment();");
}
//-----------------------------------------------------------------------
bool AttachmentComboWidget::setValue(const QString string)
{
    if (string == "" || string == "scroll" || string == "fixed"){
        comboAttachment->setCurrentText(string);
        return true;
    }
    return false;
}
//-----------------------------------------------------------------------
void AttachmentComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("attachment(" + s + ");");
    emit atas();
}
//-----------------------------------------------------------------------
// Alignment, Position,
//-----------------------------------------------------------------------
AlignmentComboWidget::AlignmentComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboAlignment1 = new QComboBox(this);
    comboAlignment1->setFrame(false);
    comboAlignment2 = new QComboBox(this);
    comboAlignment2->setFrame(false);
    QStringList alignment;
        alignment << "" << "top" << "bottom" << "left" << "right" << "center";
    comboAlignment1->addItems(alignment);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboAlignment1);
    layout->addWidget(comboAlignment2);
    setLayout(layout);
    connect(comboAlignment1, SIGNAL(currentIndexChanged(int)), this, SLOT(changeValues(int)));
    connect(comboAlignment1, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
    connect(comboAlignment2, SIGNAL(currentTextChanged(QString)), this, SLOT(SetStatustip(QString)));
    this->setAccessibleName("AlignmentCombo");
    this->setWindowTitle("alignment();");
}
//-----------------------------------------------------------------------
void AlignmentComboWidget::changeValues(int index)
{
    QStringList sl;
    sl.clear();
    comboAlignment2->clear();
    switch (index) {
    case 0: sl << "" << "top" << "bottom" << "left" << "right" << "center";
        break;
    case 1: sl << "" << "left" << "right";
        break;
    case 2: sl << "" << "left" << "right";
        break;
    case 3: sl << "" << "top" << "bottom";
        break;
    case 4: sl << "" << "top" << "bottom";
        break;
    case 5: sl.clear();
        break;
    default:
            ;
    }
    comboAlignment2->addItems(sl);
}
//-----------------------------------------------------------------------
void AlignmentComboWidget::setValues(QString values)
{
    QStringList list = values.split(" ");
    comboAlignment1->setCurrentText(list.at(0));
    if (list.count() > 1)
        comboAlignment2->setCurrentText(list.at(1));
}
//-----------------------------------------------------------------------
void AlignmentComboWidget::setStatustip(QString s)
{
    QString str;
    str = s + " " + comboAlignment2->currentText();
    str = str.trimmed();
    setStatusTip(str);
    this->setWindowTitle("alignment(" + this->statusTip() + ");");
}
//-----------------------------------------------------------------------
void AlignmentComboWidget::SetStatustip(QString s)
{
    QString str;
    str = comboAlignment1->currentText() + " " + s;
    str = str.trimmed();
    setStatusTip(str);
    setWindowTitle("alignment(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
// font-family
//-----------------------------------------------------------------------
FontComboWidget::FontComboWidget(QWidget *parent) :
    QFontComboBox(parent)
{
    comboFont = new QFontComboBox(this);
    comboFont->setFrame(false);
    QSize size(16, 16);
    comboFont->setIconSize(size);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboFont);
    setLayout(layout);
    connect(comboFont, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
}
//-----------------------------------------------------------------------
void FontComboWidget::setValue(QString string)
{
        QString str = string;
        str = str.remove('"');
        str = str.trimmed();
        comboFont->setCurrentText(str);
        setStatustip(string);
}
//-----------------------------------------------------------------------
void FontComboWidget::setStatustip(QString s)
{
  if (s == "")
      setStatusTip("");
  else
      setStatusTip("\"" + s + "\"");
    setWindowTitle("font-family(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
// font-style
//-----------------------------------------------------------------------
FontStyleComboWidget::FontStyleComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboFontStyle = new QComboBox(this);
    comboFontStyle->setFrame(false);
    QStringList style;
    style << "" << "normal" << "italic" << "oblique";
    comboFontStyle->addItems(style);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboFontStyle);
    setLayout(layout);
    connect(comboFontStyle, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
}
//-----------------------------------------------------------------------
void FontStyleComboWidget::setValue(QString string)
{
   if (!string.isEmpty()){
        QString str = string;
        str = str.trimmed();
        comboFontStyle->setCurrentText(str);
        setStatusTip(string);
    }/*  */
}
//-----------------------------------------------------------------------
void FontStyleComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("font-style(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
// font-weight
//-----------------------------------------------------------------------
FontWeightComboWidget::FontWeightComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboFontWeight = new QComboBox(this);
    comboFontWeight->setFrame(false);
    QStringList weight;
    weight << "" << "normal" << "bold" << "100" << "200" << "300" << "400"
           << "500" << "600" << "700" << "800" << "900";
    comboFontWeight->addItems(weight);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboFontWeight);
    setLayout(layout);
    connect(comboFontWeight, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
}
//-----------------------------------------------------------------------
void FontWeightComboWidget::setValue(QString string)
{
   if (!string.isEmpty()){
        QString str = string;
        str = str.trimmed();
        comboFontWeight->setCurrentText(str);
        setStatusTip(string);
    }/*  */
}
//-----------------------------------------------------------------------
void FontWeightComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("font-weight(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
// text-decoration: none | underline | overline | line-through
//-----------------------------------------------------------------------
TextDecorComboWidget::TextDecorComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboTextDecor = new QComboBox(this);
    comboTextDecor->setFrame(false);
    QStringList style;
    style << "" << "none" << "underline" << "overline" << "line-through";
    comboTextDecor->addItems(style);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboTextDecor);
    setLayout(layout);
    connect(comboTextDecor, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
}
//-----------------------------------------------------------------------
void TextDecorComboWidget::setValue(QString string)
{
   if (!string.isEmpty()){
        QString str = string;
        str = str.trimmed();
        comboTextDecor->setCurrentText(str);
        setStatusTip(string);
    }/*  */
}
//-----------------------------------------------------------------------
void TextDecorComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("decoration(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
MultiComboWidget::MultiComboWidget(QWidget *parent) :
    QComboBox(parent)
{
    comboMulti = new QComboBox(this);
    comboMulti->setFrame(false);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(comboMulti);
    setLayout(layout);
    connect(comboMulti, SIGNAL(currentTextChanged(QString)), this, SLOT(setStatustip(QString)));
}
//-----------------------------------------------------------------------
void MultiComboWidget::setValues(QString string)
{
    if (!string.isEmpty()){
        QString str = string;
        str = str.trimmed();
        QStringList sl;
        sl = str.split(",");
        comboMulti->addItems(sl);
    }
}
//-----------------------------------------------------------------------
void MultiComboWidget::setStatustip(QString s)
{
    setStatusTip(s);
    setWindowTitle("combo(" + this->statusTip() + ");");
    emit atas();
}
//-----------------------------------------------------------------------
