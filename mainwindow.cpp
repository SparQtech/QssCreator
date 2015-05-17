#include <QTranslator>
#include <QTextEdit>
#include <QLineEdit>
#include <QStringList>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QStringListModel>
#include <QStyleFactory>
#include <QTextStream>
#include <QTime>
#include <QColor>
#include <QTextDocumentWriter>
#include <QSettings>
#include <QtNetwork>
#include <QtWebKitWidgets>
#ifdef HAVE_QT5
#include <QUiLoader>
#endif
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spinwidgets.h"

//-----------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
            ui(new Ui::MainWindow), completer(0)

{
    QTranslator Translator;
    Translator.load("qsscreator_" + QLocale::system().name(),
                qApp->applicationDirPath());
        qApp->installTranslator(&Translator);

    ui->setupUi(this);
    this->setWindowIcon(QIcon(":/qsscreator.ico"));

    //Fucking Antialiasing
    QFont font;//("Verdana", 10);
    font.setStyleStrategy(QFont::NoAntialias);
    setFont(font); /**/

#ifndef HAVE_QT5
        ui->actionLoad_UI_file->setVisible(false);
#endif

    progress = 0;
    QNetworkProxyFactory::setUseSystemConfiguration(true);

    helpView = new QWebView(this);
    connect(helpView, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
    connect(helpView, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(helpView, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(helpView, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));
    connect(helpView->page(), SIGNAL(statusBarMessage(QString)), SLOT(statusBarMsg(QString)));

    locationEdit = new QLineEdit(this);
    locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
    connect(locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    helpToolBar = new QToolBar();// QIcon(":/images/open.png"), tr("&Open..."), this
    helpToolBar->addAction(helpView->pageAction(QWebPage::Back));
    helpToolBar->addAction(helpView->pageAction(QWebPage::Forward));
    helpToolBar->addAction(helpView->pageAction(QWebPage::Reload));
    helpToolBar->addAction(helpView->pageAction(QWebPage::Stop));
    //toolBar->addAction(helpView->pageAction(QWebPage::StopScheduledPageRefresh));
    helpToolBar->addWidget(locationEdit);

    QAction *setHelpPath = new QAction(QIcon(":res/help.png"), tr("Set Help Path..."), this);
    connect(setHelpPath, SIGNAL(triggered()), this, SLOT(setHelpPath()));
    helpToolBar->addAction(setHelpPath);

    labelHelpTitle = new QLabel();
    labelHelpTitle->setText(tr("Help"));
    font.setBold(true);
    labelHelpTitle->setFont(font);
    labelHelpTitle->setIndent(10);

    labelHelpStatus = new QLabel();labelHelpStatus->setText("Status");
    font.setBold(true);
    labelHelpStatus->setFont(font);
    labelHelpStatus->setIndent(10);

    QVBoxLayout *layoutHelp = new QVBoxLayout(ui->tabHelp);
    layoutHelp->setSpacing(0);
    layoutHelp->setContentsMargins(0, 0, 0, 0);
    layoutHelp->addWidget(labelHelpTitle);
    layoutHelp->addWidget(helpToolBar);
    layoutHelp->addWidget(helpView);
    layoutHelp->addWidget(labelHelpStatus);

    ui->tabWidget->setCurrentIndex(0);

    QMainWindow::setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    QMainWindow::setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    QMainWindow::setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    recentTemplateMenu = new QMenu(this);
    for (int i = 0; i < MaxRecentTemplateFiles; ++i) {
        recentTemplateFileActions[i] = new QAction(recentTemplateMenu);
        recentTemplateFileActions[i]->setIcon(QIcon(":/res/template_open.png"));
        recentTemplateFileActions[i]->setVisible(false);
        recentTemplateMenu->addAction(recentTemplateFileActions[i]);
        connect(recentTemplateFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentTemplateFile()));
    }
    ui->actionRecentStyleTemplates->setMenu(recentTemplateMenu);

    recentQssMenu = new QMenu(this);
    for (int i = 0; i < MaxRecentQssFiles; ++i) {
        recentQssFileActions[i] = new QAction(ui->actionRecentQssFiles);
        recentQssFileActions[i]->setIcon(QIcon(":/res/qss_open.png"));
        recentQssMenu->addAction(recentQssFileActions[i]);
        recentQssFileActions[i]->setVisible(false);
        connect(recentQssFileActions[i], SIGNAL(triggered()),
                this, SLOT(openRecentQssFile()));
    }
    ui->actionRecentQssFiles->setMenu(recentQssMenu);

    setupTips();

    QRegExp regExp(".(.*)\\+?Style");
    QString defaultStyle = QApplication::style()->metaObject()->className();

    if (regExp.exactMatch(defaultStyle))
        defaultStyle = regExp.cap(1);

    styleCombo = new QComboBox(ui->styleToolBar);
    ui->styleToolBar->addWidget(styleCombo);
    styleCombo->setToolTip(tr("Set Style"));
    styleCombo->addItems(QStyleFactory::keys());
    styleCombo->setCurrentIndex(styleCombo->findText(defaultStyle, Qt::MatchContains));
    connect(styleCombo, SIGNAL(activated(QString)), this, SLOT(setQStyle(QString)));

    qsssheet = new QssSheet(this);
    editorInternCss = new QssCodeEditor(this);
    editorExternCss = new QssCodeEditor(ui->dockEditorWidget);
    editorGeneral =  new QssCodeEditor(ui->tabExport);
    editorDeep = new QssCodeEditor(ui->pageDigger);

    connect(editorInternCss, SIGNAL(textChanged()),     this, SLOT(applyStyleSheet()));
    connect(editorExternCss, SIGNAL(textChanged()), this, SLOT(applyStyleSheet()));
    connect(editorGeneral, SIGNAL(textChanged()),  this, SLOT(applyStyleSheet()));

    qsshighlighter =     new QssHighlighter(editorInternCss->document());
    qsshighlighterBase = new QssHighlighter(editorExternCss->document());
    qsshighlighterGen =  new QssHighlighter(editorGeneral->document());

    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/res/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    editorInternCss->setCompleter(completer);
    editorExternCss->setCompleter(completer);
    editorGeneral->setCompleter(completer);

    QVBoxLayout *layout1 = new QVBoxLayout(ui->pageStyleSheet);
    layout1->setSpacing(0);
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->addWidget(qsssheet);
    QVBoxLayout *layout2 = new QVBoxLayout(ui->pageCodeEdit);
    layout2->setSpacing(0);
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(editorInternCss);

    ui->dockEditorWidget->setWidget(editorExternCss);

    QVBoxLayout *layout4 = new QVBoxLayout(ui->tabExport);
    layout4->setSpacing(0);
    layout4->setContentsMargins(0, 0, 0, 0);
    layout4->addWidget(editorGeneral);

    readSettings();

    connect(ui->actionAbout_Qt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    qApp->installEventFilter (this);

    rubberBand = new QRubberBand(QRubberBand::Line, this);
    rubberBand->setStyleSheet("border-color: #0000ff;border-width: 2; border-style: solid;");

    QTime t = QTime::currentTime();
    consoleMsg(tr("Starting up : ") + t.toString());
}
//-----------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//-----------------------------------------------------------------------
void MainWindow::writeSettings()
{
    QSettings settings("SparQtech", "QSS_Creator");
    settings.setValue("geometry", geometry());
    settings.setValue("recentQssFiles", recentQssFiles);
    settings.setValue("recentTemplateFiles", recentTemplateFiles);
    settings.setValue("showConsole", ui->actionShowConsole->isChecked());
    settings.setValue("windowState", saveState());
    settings.setValue("recentExQssFile", curQssFile);
    settings.setValue("recentTQssFile", curTemplateFile);
    settings.setValue("recentUiFile", curUiFile);
    settings.setValue("helpPath", helpPath);
    settings.setValue("curHelpPage", curHelpPage);
}
//-----------------------------------------------------------------------
void MainWindow::readSettings()
{
    QSettings settings("SparQtech", "QSS_Creator");
    QRect rect = settings.value("geometry",
                                    QRect(200, 0, 750, 450)).toRect();
    move(rect.topLeft());
    resize(rect.size());
    ui->actionShowConsole->setChecked(settings.value("showConsole", true).toBool());
    restoreState(settings.value("windowState").toByteArray());
    recentQssFiles = settings.value("recentQssFiles").toStringList();
    recentTemplateFiles = settings.value("recentTemplateFiles").toStringList();
    helpPath = settings.value("helpPath").toString();
    curHelpPage = settings.value("curHelpPage").toString();
    updateRecentQssFileActions();
    updateRecentTemplateFileActions();
    QFile ftqss(settings.value("recentTQssFile").toString());
    if (ftqss.exists())
        loadTemplate(ftqss.fileName());
    else
        loadTemplate(":/res/default.tqss");

    QFile fui(settings.value("recentUiFile").toString());
    if (fui.exists())
        loadUiFile(fui.fileName());
    else
        loadUiFile(":/res/default.ui");

    QFile fxqss(settings.value("recentExQssFile").toString());
    if (fxqss.exists())
        loadQssFile(fxqss.fileName());
}
//-----------------------------------------------------------------------
void MainWindow::setCurrentTemplateFileName(const QString &fileName)
{
    curTemplateFile = fileName;
    QString shownName = "new***.tqss";
    if (!curTemplateFile.isEmpty()) {
        shownName = strippedName(curTemplateFile);
        recentTemplateFiles.removeAll(curTemplateFile);
        recentTemplateFiles.prepend(curTemplateFile);
        updateRecentTemplateFileActions();
    }
    else
        curTemplateFile = "";//shownName;
    setWindowTitle(QString("%1 & %2 - %3").arg(shownName)
                                     .arg(strippedName(curQssFile))
                                     .arg(tr("QSS Creator")));
}
//-----------------------------------------------------------------------
void MainWindow::setCurrentQssFileName(const QString &fileName)
{
    curQssFile = fileName;
    QString shownName = "new***.qss";
    if (!curQssFile.isEmpty()) {
        shownName = strippedName(curQssFile);
        recentQssFiles.removeAll(curQssFile);
        recentQssFiles.prepend(curQssFile);
        updateRecentQssFileActions();
    }
    else
        curQssFile = "";

    setWindowTitle(QString("%1 :: %2 - %3").arg(strippedName(curTemplateFile))
                                     .arg(shownName)
                                     .arg(tr("QSS Creator")));
}
//-----------------------------------------------------------------------
QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
//-----------------------------------------------------------------------
void MainWindow::updateRecentTemplateFileActions()
{
    QMutableStringListIterator i(recentTemplateFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }
    for (int j = 0; j < MaxRecentTemplateFiles; ++j) {
        if (j < recentTemplateFiles.count()) {
            QString text = QString("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentTemplateFiles[j]));
            recentTemplateFileActions[j]->setText(text);
            recentTemplateFileActions[j]->setData(recentTemplateFiles[j]);
            recentTemplateFileActions[j]->setVisible(true);
        } else {
            recentTemplateFileActions[j]->setVisible(false);
        }
    }
    ui->actionRecentStyleTemplates->setVisible(!recentTemplateFiles.isEmpty());
}
//-----------------------------------------------------------------------
void MainWindow::updateRecentQssFileActions()
{
    QMutableStringListIterator i(recentQssFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }
    for (int j = 0; j < MaxRecentQssFiles; ++j) {
        if (j < recentQssFiles.count()) {
            QString text = QString("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentQssFiles[j]));
            recentQssFileActions[j]->setText(text);
            recentQssFileActions[j]->setData(recentQssFiles[j]);
            recentQssFileActions[j]->setVisible(true);
        } else {
            recentQssFileActions[j]->setVisible(false);
        }
    }
    ui->actionRecentQssFiles->setVisible(!recentQssFiles.isEmpty());
}
//-----------------------------------------------------------------------
void MainWindow::openRecentTemplateFile()
{
    if (maybeTemplateSave()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
           loadTemplate(action->data().toString());
    }
}
//-----------------------------------------------------------------------
void MainWindow::openRecentQssFile()
{
    if (maybeQssSave()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
           loadQssFile(action->data().toString());
    }
}
//-----------------------------------------------------------------------
////////////////////    UrlLineEdit   ////////////////////////////////////
//-----------------------------------------------------------------------
UrlLineEdit::UrlLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    urlLineEdit = new QLineEdit(this);
    urlLineEdit->setFrame(false);
    urlLineEdit->setPlaceholderText(tr("Enter image url"));
    urlLineEdit->setToolTip(tr("Example") + ": :/res/icon.png");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(urlLineEdit);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    connect(urlLineEdit, SIGNAL(textChanged(QString)), this, SLOT(setStatustip(QString)));
    this->setAccessibleName("image");
    setWindowTitle("url();");
}
//-----------------------------------------------------------------------
void UrlLineEdit::setValue(const QString &value)
{
    urlLineEdit->setText(value);
}
//-----------------------------------------------------------------------
void UrlLineEdit::setStatustip(QString s)
{
    if (!s.isEmpty()){
        setStatusTip("url(" + s + ")");
        setWindowTitle(statusTip() + ";");
    }
    else{
        setStatusTip("");
        setWindowTitle("url();");
    }
        emit atas();
}
//-----------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------
QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
            words << line.trimmed();
    }

    QApplication::restoreOverrideCursor();

    return new QStringListModel(words, completer);
}
//-----------------------------------------------------------------------
void MainWindow::consoleMsg(const QString &msg)
{
    ui->plainTextCons->appendPlainText(msg);
}
//-----------------------------------------------------------------------
void MainWindow::closeEvent(QCloseEvent *e)
{
    if (maybeTemplateSave() && maybeQssSave())
        e->accept();
    else
        e->ignore();
    writeSettings();
}
//-----------------------------------------------------------------------
bool MainWindow::maybeQssSave()
{
    if (!editorExternCss->document()->isModified())
        return true;

    QMessageBox::StandardButton ret;
    ret = QMessageBox::question(this, tr("QSS Creator"),
                               tr("The QSS File\n%1\nhas been modified.\n"
                                  "Do you want to save your changes?").arg(curQssFile),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return qssFileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}
//-----------------------------------------------------------------------
bool MainWindow::maybeTemplateSave()
{
    if (!bTemplateChahged)
        return true;

    if (!curTemplateFile.startsWith(":")){
        QMessageBox::StandardButton ret;
        ret = QMessageBox::question(this, tr("QSS Creator"),
                               tr("The Template File\n%1\nhas been modified.\n"
                                  "Do you want to save your changes?").arg(curTemplateFile),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return templateFileSave();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}
//-----------------------------------------------------------------------
bool MainWindow::qssFileSave()
{
    if (curQssFile.isEmpty())
        return qssFileSaveAs();
    if (curQssFile.startsWith(QStringLiteral(":/")))
        return qssFileSaveAs();

    QTextDocumentWriter writer(curQssFile, "TXT");
    bool success = writer.write(editorExternCss->document());
    if (success){
        editorExternCss->document()->setModified(false);
        ui->actionSaveExternalQssFile->setEnabled(false);
        consoleMsg(tr("Save file:\n") + curQssFile);
    }
    return success;
}
//-----------------------------------------------------------------------
bool MainWindow::qssFileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save QSS as..."), QString(),
                                              tr("QSS-files (*.qss);;TXT-files "
                                                 "(*.txt)"));
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".qss", Qt::CaseInsensitive)
          || fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".qss"; // default

    setCurrentQssFileName(fn);
    return qssFileSave();
}
//-----------------------------------------------------------------------
bool MainWindow::templateFileSave()
{
    if (curTemplateFile.isEmpty())
        return templateFileSaveAs();
    if (curTemplateFile.startsWith(QStringLiteral(":/")))
        return templateFileSaveAs();

    bool success = saveTemlateFile(curTemplateFile);
    if (success){
        bTemplateChahged = false;
        ui->actionSaveTemplate->setEnabled(false);
    }
    return success;
}
//-----------------------------------------------------------------------
bool MainWindow::templateFileSaveAs()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save Template as..."), QString(),
                                              tr("Template-files (*.tqss);;TXT-files "
                                                 "(*.txt)"));
    if (fn.isEmpty())
        return false;
    if (!(fn.endsWith(".tqss", Qt::CaseInsensitive)
          || fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".tqss"; // default

    setCurrentTemplateFileName(fn);
    return templateFileSave();
}
//-----------------------------------------------------------------------
void MainWindow::qssFileNew()
{
    if (maybeQssSave()) {
        editorExternCss->clear();
        setCurrentQssFileName(QString());
        ui->dockEditorWidget->show();
        ui->radioBtnLoadedQss->setEnabled(true);
        ui->radioBtnBothQss->setEnabled(true);
        ui->actionExportGeneralQSS->setEnabled(true);
    }
}
//-----------------------------------------------------------------------
void MainWindow::openQssFile()
{
    if (maybeQssSave()){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open QSS File"), QString(),
                                                     tr("QSS-files (*.qss);;All Files (*)"));
        if (!fileName.isEmpty())
            loadQssFile(fileName);
    }
}
//-----------------------------------------------------------------------
bool MainWindow::loadQssFile(const QString &fileName)
{
    if (!QFile::exists(fileName))
        return false;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return false;

    QTextStream in(&file);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    editorExternCss->document()->setPlainText(in.readAll());

    QApplication::restoreOverrideCursor();

    setCurrentQssFileName(fileName);
    ui->radioBtnLoadedQss->setEnabled(true);
    ui->radioBtnBothQss->setEnabled(true);
    ui->dockEditorWidget->show();
    editorExternCss->document()->setModified(false);
    ui->actionSaveExternalQssFile->setEnabled(false);
    ui->actionExportGeneralQSS->setEnabled(true);
    ui->radioBtnLoadedQss->setChecked(true);
    ui->dockEditorWidget->show();
    applyStyleSheet();

    return true;
}
//-----------------------------------------------------------------------
void MainWindow::applyStyleSheet()
{
    if (ui->groupBoxApplyQss->isChecked()){
        QString str;
        if (ui->radioBtnCreatedQss->isChecked())
            str = editorInternCss->document()->toPlainText();
        if (ui->radioBtnLoadedQss->isChecked())
            str = editorExternCss->document()->toPlainText();
        if (ui->radioBtnBothQss->isChecked())
            str = editorExternCss->document()->toPlainText() +
                  editorInternCss->document()->toPlainText();
        ui->scrollArea->setStyleSheet(str);
        if (editorExternCss->document()->isModified())
            ui->actionSaveExternalQssFile->setEnabled(true);
    }
    else
        ui->scrollArea->setStyleSheet("");
}
//-----------------------------------------------------------------------
void MainWindow::updateQssText()
{
  if (!bWait){
    bTemplateChahged = true;
    ui->actionSaveTemplate->setEnabled(true);
    editorInternCss->clear();
    QString str1, str2, sHead, sBody;
    sHead.clear();
    sBody.clear();

    for (int row = 0; row < qsssheet->rowCount(); ++row) {
        str1 = qsssheet->getText(row, 0);
        str2 = qsssheet->getText(row, 1);
        if (str2 == "-" || str2 == "+"){
            if (!sBody.isEmpty()){
                sBody = sHead + "{\n" + sBody + "}\n";
                editorInternCss->appendPlainText(sBody);//
                sBody.clear();
            }
            /*else*/ sHead = str1;
        }
        else if (str2 != "" && str2 != "off"){
            str1 = "  " + str1 + " " + str2 + ";\n";
            sBody.append(str1);
        }
        if ((row == qsssheet->rowCount()-1) && (sBody != "")){
            sBody = sHead + "{\n" + sBody + "}\n";
            editorInternCss->appendPlainText(sBody);
        }
   }
    if (ui->tabWidget->currentIndex() == 2 &&
            editorExternCss->document()->toPlainText() != "")
        updateGeneralQssText();
  }
}
//-----------------------------------------------------------------------
void MainWindow::updateGeneralQssText()
{
    editorGeneral->document()->setPlainText(editorExternCss->document()->toPlainText()
                + "/* ———————— ↑↑↑↑ external QSS ↑↑ × ↓↓ internal QSS ↓↓↓↓ ———————— */\n"
                + editorInternCss->document()->toPlainText());
}
//-----------------------------------------------------------------------
bool MainWindow::saveTemlateFile(const QString &fileName)
{
    QString strOut1, strOut2, strOut3;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("QSS Creator"),
                             tr("Cannot write file %1:\n%2.")
                              .arg(file.fileName())
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    QTextStream out(&file);

    for (int row = 0; row < qsssheet->rowCount(); ++row) {
        strOut1 = qsssheet->getText(row, 0);
        strOut2 = qsssheet->getObj(row, 1);
        strOut3 = qsssheet->getText(row, 1);
        if (strOut3 == "-" || strOut3 == "+")
            out << strOut3 + strOut1 << endl;
        else
            out << strOut1 + strOut2 << endl;
    }
    QApplication::restoreOverrideCursor();

    ui->actionSaveTemplate->setEnabled(false);
    return true;
}
//-----------------------------------------------------------------------
void MainWindow::clearTemplate()
{
  if (maybeTemplateSave()){
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    qsssheet->deleteLater();
    qsssheet = new QssSheet(this);
    ui->pageStyleSheet->layout()->addWidget(qsssheet);

    loadTemplate(":/res/default.tqss");
    setCurrentTemplateFileName("");
    QApplication::restoreOverrideCursor();
    ui->actionSaveTemplate->setEnabled(false);
  }
}
//-----------------------------------------------------------------------
void MainWindow::on_actionOpenUIFile_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                        tr("Open UI File"), "./", tr("UI Files (*.ui)"));
    if (!fileName.isEmpty())
        loadUiFile(fileName);
}
//-----------------------------------------------------------------------
void MainWindow::loadUiFile(const QString &fileName)
{
    QUiLoader loader;
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QWidget *myWidget = loader.load(&file, this);
    ui->scrollArea->setWidget(myWidget);
    curUiFile = fileName;
    file.close();
}
//-----------------------------------------------------------------------
void MainWindow::openTemplateFile()
{
  if (maybeTemplateSave()){
    QFile file(QFileDialog::getOpenFileName(this,
        tr("Open Template File"), "./", tr("Template Files (*.tqss)")));

    if (!QFile::exists(file.fileName()) && !file.fileName().isEmpty())
        QMessageBox::warning(this, tr("QSS Creator"),
                             tr("File \"%1\" not exists.").arg(file.fileName()));

    if (!file.open(QFile::ReadOnly)  && !file.fileName().isEmpty())
        QMessageBox::warning(this, tr("QSS Creator"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(file.fileName())
                             .arg(file.errorString()));
    else if (QFile::exists(file.fileName()))
        loadTemplate(file.fileName());
    }
}
//-----------------------------------------------------------------------
void MainWindow::loadTemplate(const QString &file)
{
    QFile F(file);

    if (!F.open(QFile::ReadOnly)  && !F.fileName().isEmpty())
            QMessageBox::warning(this, tr("QSS Creator"),
                                 tr("Cannot read file %1:\n%2.")
                                 .arg(F.fileName())
                                 .arg(F.errorString()));

    else{
        setCurrentTemplateFileName(F.fileName());
        QTime t;
        t.start();
        consoleMsg(tr("Load file : ") + QString(F.fileName()));
        qsssheet->deleteLater();
        qsssheet = new QssSheet(this);
        ui->pageStyleSheet->layout()->addWidget(qsssheet);
        QString line;
        int iStrNum = 0;
        QTextStream in(&F);
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        bWait = true;

        while (!in.atEnd()) {
             ++iStrNum;
             line = F.readLine();
             line = line.trimmed();
             if (line.startsWith("-"))
                 loadTemplCaption(line, true);
             else if (line.startsWith("+"))
                 loadTemplCaption(line, false);
             else if (line.contains(":"))
                 loadTemplParam(line, iStrNum);
             else if (line == "box();")
                 addBoxWidgets();
         }
        consoleMsg(tr("Time elapsed: %1 ms").arg(t.elapsed()));
        QApplication::restoreOverrideCursor();
        bWait = false;
        updateQssText();
        bTemplateChahged = false;
        ui->actionSaveTemplate->setEnabled(false);
    }
}
//-----------------------------------------------------------------------
void MainWindow::loadTemplCaption(const QString &line, bool closed)
{
    QColor color;
    QColor cap_color(QColor(64,64,64)); //Caption color
    QColor sub_cap_color(QColor(128,128,128));
    if (line.contains(":") || line.contains(" "))
        color = sub_cap_color;
    else
        color = cap_color;
    QString str, help, img = "<img src=\":/res/info.png\"> <b style=\"color:#00aa55;\"><u>";
    QString sBox = tr("Supports the <b>box model</b>.<br>");
    QString sIts = tr("Inherits");
    QString sIted = tr("Inherited By");
    QString sA = tr("and");
    str = line;
    str = str.trimmed();
    str = str.mid(1);

    if (str.contains("QWidget")){
        help = img + QString("QWidget</u></b><br>%1: <b>QObject</b> %2 <b>QPaintDevice</b>.<br>").arg(sIts).arg(sA) + tip_wid;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QAbstractScrollArea")){
        help = img + "QAbstractScrollArea</u></b><br>" + sBox + tip_asa;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QAbstractItemView")){
        help = img + "QAbstractItemView</u></b><br>" + sBox + tip_aiv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QAbstractButton")){
        help = img + "QAbstractButton</u></b><br>" + sBox + tip_abt;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QAbstractSlider")){
        help = img + "QAbstractSlider</u></b><br>" + sBox + tip_abs;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QAbstractSpinBox")){
        help = img + QString("QAbstractSpinBox</u></b><br>%1: <b>QDateTimeEdit, QDoubleSpinBox</b> \
                %2 <b>QSpinBox</b>.<br>").arg(sIted).arg(sA) + sBox + tip_spb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QCheckBox")){
        help = img + QString("QCheckBox</u></b><br>%1: <b>QAbstractButton</b>.<br>").arg(sIts) + sBox + tip_chb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QColumnView")){
        help = img + "QColumnView</u></b><br>" + tip_clv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QComboBox")){
        help = img + "QComboBox</u></b><br>"  + tip_cbb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDateEdit")){
        help = img + QString("QDateEdit</u></b><br>%1: <b>QDateTimeEdit</b>.").arg(sIts) + sBox + tip_spb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDateTimeEdit")){
        help = img + QString("QDateTimeEdit</u></b><br>%1: <b>QAbstractSpinBox</b>.<br>%2: \
                        <b>QDateEdit</b> %3 <b>QTimeEdit</b>.<br>").arg(sIts).arg(sIted).arg(sA) + tip_spb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDialogButtonBox")){
        help = img + tr("QDialogButtonBox</u></b><br>Inherits: <b>QWidget</b>.<br>") + tip_dbb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDialog")){ //NB! "QDialogButtonBox" contains "QDialog"
        help = img + QString("QDialog</u></b><br>%1: <b>QWidget</b>.<br>%2: <b>QColorDialog, \
                        QErrorMessage, QFileDialog, QFontDialog, QInputDialog, QMessageBox, \
                        QProgressDialog</b>, %3 <b>QWizard</b>.<br>").arg(sIts).arg(sIted).arg(sA) + tip_dlg;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDockWidget")){
        help = img + QString("QDockWidget</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_dkw;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QDoubleSpinBox")){
        help = img + QString("QDoubleSpinBox</u></b><br>%1: <b>QAbstractSpinBox</b>.<br>").arg(sIts) + tip_spb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QFrame")){
        help = img + QString("QFrame</u></b><br>%1: <b>QWidget</b>.<br>%2: <b>QAbstractScrollArea, \
                        QLabel, QLCDNumber, QSplitter, QStackedWidget</b>, %3 <b>QToolBox</b>. \
                        <br>").arg(sIts).arg(sIted).arg(sA) + sBox + tip_frm;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QGroupBox")){
        help = img + QString("QGroupBox</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + sBox + tip_grb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QHeaderView")){
        help = img + QString("QHeaderView</u></b><br>%1: <b>QAbstractItemView</b>.<br>").arg(sIts) + sBox + tip_hdv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QLabel")){
        help = img + QString("QLabel</u></b><br>%1: <b>QFrame</b>.<br>").arg(sIts) + sBox + tip_lbl;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QLineEdit")){
        help = img + QString("QLineEdit</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + sBox + tip_lne;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QListView")){
        help = img + QString("QListView</u></b><br>%1: <b>QAbstractItemView</b>.<br>%2: \
                        <b>QListWidget</b> %3 <b>QUndoView</b>.<br>").arg(sIts).arg(sIted).arg(sA) + sBox + tip_lsv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QListWidget")){
        help = img + QString("QListWidget</u></b><br>%1: <b>QListView.</b>.<br>").arg(sIts) + sBox + tip_lsv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QMainWindow")){
        help = img + QString("QMainWindow</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_mwd;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QMenuBar")){
        help = img + QString("QMenuBar</u></b><br>%1: <b>QWidget.</b>.<br>").arg(sIts) + sBox + tip_mnb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QMenu")){
        help = img + QString("QMenu</u></b><br>%1: <b>QWidget.</b>.<br>").arg(sIts) + sBox + tip_mnu;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QMessageBox")){
        help = img + QString("QMessageBox</u></b><br>%1: <b>QDialog.</b>.<br>").arg(sIts) + tip_mbx;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QProgressBar")){
        help = img + QString("QProgressBar</u></b><br>%1: <b>QWidget.</b>.<br>").arg(sIts) + sBox + tip_prb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QPushButton")){
        help = img + QString("QPushButton</u></b><br>%1: <b>QAbstractButton.</b>.<br>%2: \
                        <b>QCommandLinkButton</b>.<br>").arg(sIts).arg(sIted) + sBox + tip_pbt;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QRadioButton")){
        help = img + QString("QRadioButton</u></b><br>%1: <b>QAbstractButton.</b>.<br>").arg(sIts) + sBox + tip_cbt;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QScrollBar")){
        help = img + QString("QScrollBar</u></b><br>%1: <b>QAbstractSlider.</b>.<br>").arg(sIts) + sBox + tip_scr;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QSizeGrip")){
        help = img + QString("QSizeGrip</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_sgr;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QSlider")){
        help = img + QString("QSlider</u></b><br>%1: <b>QAbstractSlider</b>.<br>").arg(sIts) + sBox + tip_sld;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QSpinBox")){
        help = img + QString("QSpinBox</u></b><br>%1: <b>QAbstractSpinBox</b>.<br>").arg(sIts) + sBox + tip_spb;
        qsssheet->addCaption(str, help, color, true);
    }
    else if (str.contains("QSplitter")){
        help = img + QString("QSplitter</u></b><br>%1: <b>QFrame</b>.<br>").arg(sIts) + sBox + tip_spl;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QStatusBar")){
        help = img + QString("QStatusBar</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_stb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTabBar")){
        help = img + QString("QTabBar</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_tbr;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTabWidget")){
        help = img + QString("QTabWidget</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + tip_tbw;
        qsssheet->addCaption(str, help, color, true);
    }
    else if (str.contains("QTableView")){
        help = img + QString("QTableView</u></b><br>%1: <b>QAbstractItemView</b>.<br>%2: \
                        <b>QTableWidget</b>.<br>").arg(sIts).arg(sIted) + sBox + tip_tbv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTableWidget")){
        help = img + QString("QTableWidget</u></b><br>%1: <b>QTableView</b>.<br>").arg(sIts) + sBox + tip_tbv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTextEdit")){
        help = img + QString("QTextEdit</u></b><br>%1: <b>QAbstractScrollArea</b>.<br>%2: \
                        <b>QTextBrowser</b>.<br>").arg(sIts).arg(sIted) + sBox + tip_txe;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTimeEdit")){
        help = img + QString("QTimeEdit</u></b><br>%1: <b>QDateTimeEdit</b>.<br>").arg(sIts) + tip_spb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QToolBar")){
        help = img + QString("QToolBar</u></b><br>%1: <b>QWidget</b>.<br>").arg(sIts) + sBox + tip_tlb;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QToolButton")){
        help = img + QString("QToolButton</u></b><br>%1: <b>QAbstractButton</b>.<br>").arg(sIts) + sBox + tip_tbt;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QToolBox")){
        help = img + QString("QToolBox</u></b><br>%1: <b>QFrame</b>.<br>").arg(sIts) + sBox + tip_tbx;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QToolTip")){
        help = img + "QToolTip</u></b><br>" + sBox + tip_ttp;
        qsssheet->addCaption(str, help, color, true);
    }
    else if (str.contains("QTreeView")){
        help = img + QString("QTreeView</u></b><br>%1: <b>QAbstractItemView</b>.<br>%2: <b>QTreeWidget</b>.<br>").arg(sIts).arg(sIted) + sBox + tip_trv;
        qsssheet->addCaption(str, help, color, closed);
    }
    else if (str.contains("QTreeWidget")){
        help = img + QString("QTreeWidget</u></b><br>%1: <b>QTreeView</b>.<br>").arg(sIts) + sBox + tip_trv;
        qsssheet->addCaption(str, help, color, closed);
    }
    //if (str == "-*" || str == "+*"){
    else if (str == "*"){
        help = img + "</u> *</b><br>" + tr("Unless otherwise specified, properties below apply to all widgets");
        qsssheet->addCaption(str, help, color, true);
    }
    else {
        help = img + str + "</b></u><br>" + tr("Unknown or custom widget");
        qsssheet->addCaption(str, help, color, true);
    }
}
//-----------------------------------------------------------------------
void MainWindow::loadTemplParam(const QString &line, int strNum)
{
    QString sName, sParam, sValues, sCurrent, str = line, stmp;
   QStringList sl = str.split(":");
    sName = sl[0];
    sParam = sl[1];
    sParam =  sParam.trimmed();

    if (sParam == "url("){ //FIXME
        sParam += ":";
        sParam += sl[2];
    }
    sName = sName.trimmed();
    sName = sName.append(":");

    sCurrent = sParam.section(')', 1, 1);
    sCurrent = sCurrent.section(';', 0, 0);
    sCurrent = sCurrent.trimmed();
    str =  sParam.left(sParam.indexOf("("));

    stmp = sParam;
    if (!stmp.remove(" ").contains("()")){
        sValues = sParam.section('(', 1, 1);
        sValues = sValues.section(')', 0, 0);
        sValues = sValues.trimmed();
    }
    if (str == "int")
            createSpinWidget(sName, sParam, strNum);

    else if (str == "color"  || str == "qlineargradient" || str == "qradialgradient"
                             || str == "qconicalgradient" ){
        sParam =  line;
        sParam = sParam.section('(', 1, 1);
        sParam = sParam.section(')', 0, 0);
        createColorWidget(sName, str, sParam, strNum);
        }
    else if (str == "repeat"){
            RepeatComboWidget *repeat;
            qsssheet->addPropertyWidget(sName, repeat = new RepeatComboWidget(this));
            connect(repeat, SIGNAL(atas()), this, SLOT(updateQssText()));
            repeat->setValue(sValues);
    }
    else if (str == "combo"){
            MultiComboWidget *combo;
            qsssheet->addPropertyWidget(sName, combo = new MultiComboWidget(this));
            connect(combo, SIGNAL(atas()), this, SLOT(updateQssText()));
            combo->setValues(sValues);
    }
    else if (str == "borderstyle"){
            BorderStyleComboWidget *borderstyle;
            qsssheet->addPropertyWidget(sName, borderstyle = new BorderStyleComboWidget(this));
            connect(borderstyle, SIGNAL(atas()), this, SLOT(updateQssText()));
            if (!borderstyle->setValue(sValues)){
                consoleMsg(tr("Wrong papameter at line : %1").arg(strNum));
                consoleMsg(tr("Parameter : ") + sParam + "\n-------------------");
            }
    }
    else if (str == "attachment"){
            AttachmentComboWidget *attach;
            qsssheet->addPropertyWidget(sName, attach = new AttachmentComboWidget(this));
            connect(attach, SIGNAL(atas()), this, SLOT(updateQssText()));
            if (!attach->setValue(sValues)){
                consoleMsg(tr("Wrong papameter at line : %1").arg(strNum));
                consoleMsg(tr("Parameter : ") + sParam + "\n-------------------");
            }
    }
    else if (str == "alignment"){
            AlignmentComboWidget *pos;
            qsssheet->addPropertyWidget(sName, pos = new AlignmentComboWidget(this));
            connect(pos, SIGNAL(atas()), this, SLOT(updateQssText()));
            pos->setValues(sValues);
    }
    else if (str == "origin"){
            OriginComboWidget *origin;
            qsssheet->addPropertyWidget(sName, origin = new OriginComboWidget(this));
            connect(origin, SIGNAL(atas()), this, SLOT(updateQssText()));
            origin->setValue(sValues);
    }
    else if (str == "font-family"){
            FontComboWidget *font;
            qsssheet->addPropertyWidget(sName, font = new FontComboWidget(this));
            connect(font, SIGNAL(atas()), this, SLOT(updateQssText()));
            font->setValue(sValues);
    }
    else if (str == "font-style"){
            FontStyleComboWidget *style;
            qsssheet->addPropertyWidget(sName, style = new FontStyleComboWidget(this));
            connect(style, SIGNAL(atas()), this, SLOT(updateQssText()));
            style->setValue(sValues);
    }
    else if (str == "font-weight"){
            FontWeightComboWidget *weight;
            qsssheet->addPropertyWidget(sName, weight = new FontWeightComboWidget(this));
            connect(weight, SIGNAL(atas()), this, SLOT(updateQssText()));
            weight->setValue(sValues);
    }
    else if (str == "decoration"){
            TextDecorComboWidget *decor;
            qsssheet->addPropertyWidget(sName, decor = new TextDecorComboWidget(this));
            connect(decor, SIGNAL(atas()), this, SLOT(updateQssText()));
            decor->setValue(sValues);
    }
    else if (str == "url"){
            UrlLineEdit *url;
            qsssheet->addPropertyWidget(sName, url = new UrlLineEdit(this));
            connect(url, SIGNAL(atas()), this, SLOT(updateQssText()));
            url->setValue(sValues);
    }
    else if (str == "box")
        addBoxWidgets();
    else if (str == "font")
        addFontWidgets();
}
//-----------------------------------------------------------------------
void MainWindow::createSpinWidget(const QString &sName, const QString &sParam, int strNum)
{
    QString Name = sName,
            Param = sParam,
            Values, Current, suffix;
    int min, max, value = 0, tmp, paramCount;
    bool ok, off = false;
    QStringList list;
    SSpinBox *spin;
    Current = Param.section(')', 1, 1);
    if (Current.contains("[")){
        suffix =  Current.section('[', 1, 1);
        if (suffix.contains("]"))
            suffix = suffix.section(']', 0, 0);
        Current = Current.section('[', 0, 0);
    }
    else
        Current = Current.section(';', 0, 0);
    Current = Current.trimmed();
        Values = Param.section('(', 1, 1);
        Values = Values.section(')', 0, 0);
        Values = Values.remove(" ");

    if (Values == ""){
        min = -99;
        max =  99;
    }
    if (Values.contains(",")){
        list = Values.split(",");
        paramCount = list.count();
        if (paramCount == 2){
            min = list[0].toInt(&ok, 10);
            if (!ok) goto err;
            max = list[1].toInt(&ok, 10);
            if (!ok)  goto err;
            if (min > max){
                tmp = min;
                min = max;
                max = tmp;
            }
            if (paramCount > 2) goto err;
        }
    }
    else if(Values != ""){
            tmp = Values.toInt(&ok, 10);
            if (!ok) goto err;
            if (tmp < 0){
                min = tmp;
                max = 0;
            }
            if (tmp > 0){
                min = 0;
                max = tmp;
            }
            if (tmp == 0)  goto err;
     }

        if (Current == "off" || Current == "") // !!!
            off = true;
        else {//if (Current != ""){
            value = Current.toInt(&ok, 10);
            if (!ok) goto err;
            if (value < min || value > max){
                consoleMsg(tr("-------------------\nCurrent value is out of the range!"));
                goto err;
            }
        }
        qsssheet->addPropertyWidget(sName, spin = new SSpinBox(this));
        connect(spin, SIGNAL(atas()), this, SLOT(updateQssText()));
        spin->setValues(min, max, value, suffix, off);
        goto pass;
        err:
        consoleMsg(tr("Wrong papameter at line : %1").arg(strNum));
        consoleMsg(tr("Parameter : ") + sParam + "\n-------------------");
        pass:
        ;
}
//-----------------------------------------------------------------------
void MainWindow::createColorWidget(const QString &sName, const QString &sType,
                                   const QString &sParam, int strNum)
{
    QString Name = sName,
            Param = sParam,
            Type =sType;
    ColorWidget *col;

    if (Type == "color"){
        qsssheet->addPropertyWidget(Name, col = new ColorWidget(qsssheet));
        connect(col, SIGNAL(atas()), this, SLOT(updateQssText()));
        if (QColor::isValidColor(Param)){
            col->colorChanged(Param);
            col->colorLineEdit->setText(Param);
        }
        else if (Param != ""){
            consoleMsg(tr("Wrong papameter at line : %1").arg(strNum));
            consoleMsg(tr("Parameter : ") + sParam + "\n --------------------");
        }
    }
    else if (Type == "qlineargradient" || Type == "qradialgradient" ||Type == "qconicalgradient"){
        qsssheet->addPropertyWidget(Name, col = new ColorWidget(qsssheet));
        col->colorLineEdit->hide();
        col->widgetGradient->show();
        col->widgetGradient->setStyleSheet("background: " + Type + "(" + Param + ");");
        col->setStatusTip(Type + "(" + Param + ")");
        col->setWindowTitle(col->statusTip() + ";");
        connect(col, SIGNAL(atas()), this, SLOT(updateQssText()));
    }
}
//-----------------------------------------------------------------------
void MainWindow::addBoxWidgets()
{
    createSpinWidget("margin:", "(-1, 99)off", -1);
    createSpinWidget("border-width:", "(-1, 99)off", -1);
    createSpinWidget("border-radius:", "(-1, 99)off", -1);

    BorderStyleComboWidget *borderstyle;
    qsssheet->addPropertyWidget("border-style:", borderstyle = new BorderStyleComboWidget(this));
    connect(borderstyle, SIGNAL(atas()), this, SLOT(updateQssText()));
    borderstyle->setValue("");

    createColorWidget("border-color:", "color", "", -1);

    UrlLineEdit *url;
    qsssheet->addPropertyWidget("border-image:", url = new UrlLineEdit(this));
    connect(url, SIGNAL(atas()), this, SLOT(updateQssText()));
    url->setValue("");

    createSpinWidget("padding:", "(-1, 99)off", -1);

    createColorWidget("background-color:", "color", "", -1);

    qsssheet->addPropertyWidget("background-image:", url = new UrlLineEdit(this));
    connect(url, SIGNAL(atas()), this, SLOT(updateQssText()));
    url->setValue("");

    RepeatComboWidget *repeat;
    qsssheet->addPropertyWidget("background-repeat:", repeat = new RepeatComboWidget(this));
    connect(repeat, SIGNAL(atas()), this, SLOT(updateQssText()));
    repeat->setValue("");

    AlignmentComboWidget *pos;
    qsssheet->addPropertyWidget("background-position:", pos = new AlignmentComboWidget(this));
    connect(pos, SIGNAL(atas()), this, SLOT(updateQssText()));
    pos->setValues("");

    OriginComboWidget *origin;
    qsssheet->addPropertyWidget("background-origin:", origin = new OriginComboWidget(this));
    connect(origin, SIGNAL(atas()), this, SLOT(updateQssText()));
    origin->setValue("");

    qsssheet->addPropertyWidget("image:", url = new UrlLineEdit(this));
    connect(url, SIGNAL(atas()), this, SLOT(updateQssText()));
    url->setValue("");

    qsssheet->addPropertyWidget("image-position:", pos = new AlignmentComboWidget(this));
    connect(pos, SIGNAL(atas()), this, SLOT(updateQssText()));
    pos->setValues("");

}
//-----------------------------------------------------------------------
void MainWindow::addFontWidgets()
{
    FontComboWidget *font;
    qsssheet->addPropertyWidget("font-family:", font = new FontComboWidget(this));
    connect(font, SIGNAL(atas()), this, SLOT(updateQssText()));
    font->setValue("");

    createSpinWidget("font-size:", "(-1, 99)off[px]", -1);

    FontStyleComboWidget *style;
    qsssheet->addPropertyWidget("font-style:", style = new FontStyleComboWidget(this));
    connect(style, SIGNAL(atas()), this, SLOT(updateQssText()));
    style->setValue("");

    FontWeightComboWidget *weight;
    qsssheet->addPropertyWidget("font-weight:", weight = new FontWeightComboWidget(this));
    connect(weight, SIGNAL(atas()), this, SLOT(updateQssText()));
    weight->setValue("");

    createColorWidget("color:", "color", "", -1);
}
//-----------------------------------------------------------------------
bool MainWindow::eventFilter( QObject *obj, QEvent *evt )
{
    if(ui->tabWidget->currentIndex() == 0){
      if(evt->type() == QEvent::MouseMove){
        QMouseEvent *m = static_cast<QMouseEvent *>(evt);
        QWidget *wid = QApplication::widgetAt(m->globalPos());
        QSize wSize;
        QPoint wPoint;
        QPoint dPoint(this->geometry().x(), this->geometry().y());
        QString sOName, sCName, sPName, sSizeW, sSizeH;
        QPoint pos = ui->scrollArea->viewport()->mapToGlobal(QPoint(0,0));
        QSize size(ui->scrollArea->width(), ui->scrollArea->height());
        QRect wRect(pos, size);
        if(wRect.contains(m->globalPos())){
          if( wid ){
              sCName = wid->metaObject()->className();
            sOName = wid->objectName();
            if (!wid->isTopLevel())
              sPName = wid->parent()->metaObject()->className();
            else
              sPName = tr("TopLevel");
              wPoint = wid->mapToGlobal(QPoint(0,0));
              wPoint = wPoint - dPoint;
              wSize = wid->size();
              if (ui->checkBoxHighlight->isChecked()){
                  rubberBand->setGeometry(QRect(wPoint, wSize));
                  rubberBand->show();
              }

            sSizeW.setNum(wSize.width());
            sSizeH.setNum(wSize.height());
            ui->labelClasses->setText("<font color=green>" + sPName + "</font>-><font color=blue>" +
                                      sCName + "</font> (" + sOName +") [" + sSizeW + " x " + sSizeH+ "px]");
            editorDeep->setPlainText(wid->styleSheet());
           }
        }
        else{
            ui->labelClasses->setText(tr("Cursor is out of the Work Area!"));
            rubberBand->hide();
        }
      }
    }
    return QMainWindow::eventFilter(obj, evt);
}
//-----------------------------------------------------------------------
void MainWindow::adjustLocation()
{
    locationEdit->setText(helpView->url().toString());
}
//-----------------------------------------------------------------------
void MainWindow::changeLocation()
{
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    helpView->load(url);
    helpView->setFocus();
}
//-----------------------------------------------------------------------
void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100)
        labelHelpTitle->setText(helpView->title());
    else
        labelHelpTitle->setText(QString("%1 (%2%)").arg(helpView->title()).arg(progress));
}
//-----------------------------------------------------------------------
void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}
//-----------------------------------------------------------------------
void MainWindow::finishLoading(bool)
{
    progress = 100;
    adjustTitle();
    curHelpPage = locationEdit->text();
}
//-----------------------------------------------------------------------
void MainWindow::statusBarMsg(QString s)
{
    labelHelpStatus->setText(s);
    consoleMsg(s);
}
//-----------------------------------------------------------------------
void MainWindow::setupTips()
{
    /// QAbstractScrollArea
    tip_asa = tr("All derivatives of <b>QAbstractScrollArea</b>, including <b>QTextEdit</b>, and <b>QAbstractItemView</b> (all item view classes), support scrollable backgrounds using background-attachment. Setting the background-attachment to fixed provides a background-image that does not scroll with the viewport. Setting the background-attachment to scroll, scrolls the background-image when the scroll bars move.");
    /// QAbstractItemView
    tip_aiv = tr("Inherited By: <b>QColumnView, QHeaderView, QListView, QTableView</b>, and <b>QTreeView</b>");
    ///QAbstractButton
    tip_abt = tr("Inherited By: <b>QCheckBox, QPushButton, QRadioButton</b>, and <b>QToolButton</b>.");
    ///QAbstractSlider
    tip_abs = tr("Inherited By: <b>QDial, QScrollBar</b>, and <b>QSlider</b>.");
    ///QCheckBox
    tip_chb = tr("The check indicator can be styled using the <b>::indicator</b> subcontrol. By default, the indicator is  placed in the Top Left corner of the Contents rectangle of the widget.<br> The spacing property specifies the spacing between the check indicator and the text.");
    ///QColumnView
    tip_clv = tr("Inherits: <b>QAbstractItemView</b>.<br>The grip can be styled be using the <b>image</b> property. The arrow indicators can by styled using the <b>::left-arrow</b> subcontrol and the <b>::right-arrow</b> subcontrol.");
    ///QComboBox
    tip_cbb = tr("Inherits: <b>QWidget</b>.<br>Inherited By: <b>QFontComboBox</b>.<br>The frame around the combobox can be styled using the <b>box model</b>. The drop-down button can be styled using the <b>::drop-down</b> subcontrol. By default, the drop-down button is placed in the top right corner of the padding rectangle of the widget. The arrow mark inside the drop-down button can be styled using the <b>::down-arrow</b> subcontrol. By default, the arrow is placed in the center of the contents rectangle of the drop-down subcontrol.<br>The pop-up of the QComboBox is a <b>QAbstractItemView</b> and is styled using the descendant selector.");
    ///For QAbstractSpinBox, QDateEdit, QDateTimeEdit, QDoubleSpinBox, and QSpinBox
    tip_spb = tr("The frame of the spin box can be styled using the <b>box model</b>.<br> The up button and arrow can be styled using the <b>::up-button</b> and <b>::up-arrow</b> subcontrols. By default, the up-button is placed in the top right corner in the Padding rectangle of the widget. Without an explicit size, it occupies half the height of its reference rectangle. The up-arrow is placed in the center of the Contents rectangle of the up-button.<br> The down button and arrow can be styled using the <b>::down-button</b> and <b>::down-arrow</b> subcontrols. By default, the down-button is placed in the bottom right corner in the Padding rectangle of the widget. Without an explicit size, it occupies half the height of its reference rectangle. The bottom-arrow is placed in the center of the Contents rectangle of the bottom-button.");
    ///QDialog
    tip_dlg = tr("Supports only the <b>background, background-clip</b> and <b>background-origin</b> properties.<br><b style=\"color:#ff0000;\">Warning</b>: Make sure you define the <b style=\"color:#00aa00;\">Q_OBJECT</b> macro for your custom widget.");
    ///QDialogButtonBox
    tip_dbb = tr("The layout of buttons can be altered using the <b>button-layout</b> property.The possible values are <b>0 (WinLayout), 1 (MacLayout), 2 (KdeLayout),</b> and <b>3 (GnomeLayout)</b>.<br>If this property is not specified, it defaults to the value specified by the current style for the <b>SH_DialogButtonLayout</b>  style hint.");
    ///QDockWidget
    tip_dkw = tr("Supports styling of the title bar and the title bar buttons when docked.<br>The dock widget border can be styled using the <b>border</b> property. The <b>::title</b> subcontrol can be used to customize the title bar. The close and float buttons are positioned with respect to the ::title subcontrol using the <b>::close-button</b> and <b>::float-button</b> respectively.<br>When the title bar is vertical, the <b>:vertical</b> pseudo class is set. In addition, depending on <b>QDockWidget::DockWidgetFeature</b>, the <b>:closable, :floatable</b> and <b>:movable</b> pseudo states are set.<br><b style=\"color:#00aa00;\">Note</b>: Use <b>QMainWindow::separator</b> to style the resize handle.<br><b style=\"color:#ff0000;\">Warning</b>: The style sheet has no effect when the QDockWidget is undocked as Qt uses native  top level windows when undocked.");
    ///QFrame
    tip_frm = tr("Since <b>4.3</b>, setting a stylesheet on a QLabel automatically sets the  <b>QFrame::frameStyle</b> property to <b>QFrame::StyledPanel</b>.");
    ///QGroupBox
    tip_grb = tr("The title can be styled using the <b>::title</b> subcontrol. By default, the title is placed depending on <b>QGroupBox::textAlignment</b>.<br>In the case of a checkable <b>QGroupBox</b>, the title includes the check indicator. The indicator is styled using the <b>::indicator</b> subcontrol. The spacing property can be used to control the spacing between the text and indicator.");
    ///QHeaderView
    tip_hdv = tr("The sections of the header view are styled using the <b>::section</b> sub control. The section Sub-control supports the <b>:middle, :first, :last, :only-one, :next-selected, :previous-selected, :selected</b>, and <b>:checked</b> pseudo states.<br>Sort indicator in can be styled using the <b>::up-arrow</b> and the <b>::down-arrow</b> Sub-control.");
    ///QLabel
    tip_lbl = tr("Does not support the <b>:hover</b> pseudo-state.<br>Since <b>4.3</b>, setting a stylesheet on a QLabel automatically sets the <b>QFrame::frameStyle</b> property to <b>QFrame::StyledPanel</b>.");
    ///QLineEdit
    tip_lne = tr("The color and background of the selected item is styled using <b>selection-color</b> and <b>selection-background-color</b> respectively.<br>The password character can be styled using the <b>lineedit-password-character</b> property.");
    ///QListView, QListWidget
    tip_lsv = tr("When <b>alternating row colors</b> is enabled, the alternating colors can be styled using the <b>alternate-background-color</b> property.<br>The color and background of the selected item is styled using <b>selection-color</b> and <b>selection-background-color</b> respectively.<br>The selection behavior is controlled by the <b>show-decoration-selected</b> property.<br>Use the <b>::item</b> subcontrol for more fine grained control over the items in the <b>QListView</b>.");
    ///QMainwindow
    tip_mwd = tr("Supports styling of the separator</b>.<br>The separator in a QMainWindow when using <b>QDockWidget</b> is styled using the <b>::separator</b> subcontrol.");
    ///QMenu
    tip_mnu = tr("Individual items are styled using the <b>::item</b> subcontrol. In addition to the usually supported pseudo states, item subcontrol supports the <b>:selected, :default, :exclusive</b> and the <b>non-exclusive</b> pseudo states.<br>The indicator of checkable menu items is styled using the <b>::indicator</b> subcontrol.<br>The separator is styled using the <b>::separator</b> subcontrol.<br>For items with a sub menu, the arrow marks are styled using the <b>right-arrow</b> and <b>left-arrow</b>.<br>The scroller is styled using the <b>::scroller</b>.<br>The tear-off is styled using the <b>::tearoff</b>.");
    ///QMenuBar
    tip_mnb = tr("The <b>spacing</b> property specifies the spacing between menu items. Individual items are styled using the <b>::item</b> subcontrol.<br> <b style=\"color:#ff0000;\"> Warning:</b> When running on Qt/Mac, the menu bar is usually embedded into the system-wide menu bar. In this case, the style sheet will have no effect.");
    ///QMessageBox
    tip_mbx = tr("The <b>messagebox-text-interaction-flags</b> property can be used to alter the interaction with text in the message box.<br>Possible values are based on <b>Qt::TextInteractionFlags</b>. <br>If this property is not specified, it defaults to the value specified by the current style for the <b>SH_MessageBox_TextInteractionFlags</b> style hint.");
    ///QProgressBar
    tip_prb = tr("The chunks of the progress bar can be styled using the <b>::chunk</b> subcontrol. The chunk is displayed on the Contents rectangle of the widget.<br> If the progress bar displays text, use the <b>text-align</b> property to position the text.<br> Indeterminate progress bars have the <b>:indeterminate</b> pseudo state set.");
    ///QPushButton
    tip_pbt = tr("Supports the <b>:default, :flat, :checked</b> pseudo states. <br>For QPushButton with a menu, the menu indicator is styled using the <b>::menu-indicator</b> subcontrol. Appearance of checkable push buttons can be customized using the <b>:open</b> and <b>:closed</b> pseudo-states.<br><br><b style=\"color:#ff0000;\"> Warning:</b> If you only set a background-color on a QPushButton, the background may not appear unless you set the border property to some value. This is because, by default, the QPushButton draws a native border which completely overlaps the background-color. For example:<br><br>QPushButton {<br> background-color: red;<br> border: none;<br>}");
    ///QRadioButton
    tip_cbt = tr("The check indicator can be styled using the <b>::indicator</b> subcontrol. By default, the indicator is placed in the Top Left corner of the Contents rectangle of the widget.<br>The <b>spacing</b> property specifies the spacing between the check indicator and the text.");
    ///QScrollBar
    tip_scr = tr("The Contents rectangle of the widget is considered to be the groove over which the slider moves. The extent of the <b>QScrollBar</b> (i.e the <b>width</b> or the <b>height</b> depending on the orientation) is set using the width or height property respectively. To determine the orientation, use the <b>:horizontal</b> and the <b>:vertical</b> pseudo states.<br>The slider can be styled using the <b>::handle</b> subcontrol. Setting the <b>min-width</b> or <b>min-height</b> provides size contraints for the slider depending on the orientation.<br>The <b>::add-line</b> subcontrol can be used to style the button to add a line. By default, the add-line subcontrol is placed in top right corner of the Border rectangle of the widget. Depending on the orientation the <b>::right-arrow</b> or <b>::down-arrow</b>. By default, the arrows are placed in the center of the Contents rectangle of the add-line subcontrol.<br>The <b>::sub-line</b> subcontrol can be used to style the button to subtract a line. By default, the sub-line subcontrol is placed in bottom right corner of the Border rectangle of the widget. Depending on the orientation the <b>::left-arrow</b> or <b>::up-arrow</b>. By default, the arrows are placed in the center of the Contents rectangle of the sub-line subcontrol.<br>The <b>::sub-page</b> subcontrol can be used to style the region of the slider that subtracts a page. The <b>::add-page</b> subcontrol can be used to style the region of the slider that adds a page.");
    ///QSizeGrip
    tip_sgr = tr("Supports the <b>width, height</b>, and <b>image</b> properties.");
    ///QSlider
    tip_sld = tr("For horizontal slides, the <b>min-width</b> and <b>height</b> properties must be provided. For vertical sliders, the <b>min-height</b> and <b>width</b> properties must be provided.<br> The groove of the slider is styled using the <b>::groove</b>. The groove is positioned by default in the Contents rectangle of the widget. The thumb of the slider is styled using <b>::handle</b> subcontrol. The subcontrol moves in the Contents rectangle of the groove subcontrol.");
    ///QSplitter
    tip_spl = tr("The handle of the splitter is styled using the <b>::handle</b> subcontrol.");
    ///QStatusBar
    tip_stb = tr("Supports only the <b>background</b> property. The frame for individual items can be style using the <b>::item</b> subcontrol.");
    ///QTabBar
    tip_tbr = tr("Individual tabs may be styled using the <b>::tab</b> subcontrol. Close buttons using the <b>::close-button</b> The tabs support the <b>:only-one, :first, :last, :middle, :previous-selected, :next-selected, :selected</b> pseudo states.<br>The <b>:top, :left, :right, :bottom</b> pseudo states depending on the orientation of the tabs.<br>Overlapping tabs for the selected state are created by using negative margins or using the absolute position scheme.<br> The tear indicator of the <b>QTabBar</b> is styled using the <b>::tear</b> subcontrol.<br><b> QTabBar</b> used two QToolButtons for its scrollers that can be styled using the QTabBar QToolButton selector. To specify the width of the scroll button use the <b>::scroller</b> subcontrol.<br>The alignment of the tabs within the <b>QTabBar</b> is styled using the <b> alignment</b> property.<br><b style=\"color:#ff0000;\">Warning</b>: To change the position of the <b>QTabBar</b> within a <b>QTabWidget</b>, use the <b>tab-bar</b> subcontrol (and set <b>subcontrol-position</b>).");
    ///QTabWidget
    tip_tbw = tr("The frame of the tab widget is styled using the <b>::pane</b> subcontrol. The left and right corners are styled using the <b>::left-corner</b> and <b>::right-corner</b> respectively. The position of the tab bar is controlled using the <b>::tab-bar</b> subcontrol.<br>By default, the subcontrols have positions of a <b>QTabWidget</b> in the QWindowsStyle. To place the <b>QTabBar</b> in the center, set the subcontrol-position of the tab-bar subcontrol.<br> The <b>:top, :left, :right, :bottom</b> pseudo states depending on the orientation of the tabs.");
    ///QTableView
    tip_tbv = tr("When <b>alternating row colors</b> is enabled, the alternating colors can be styled using the <b>alternate-background-color</b> property.<br>The color and background of the selected item is styled using <b>selection-color</b> and <b>selection-background-color</b> respectively.<br>The corner widget in a </b>QTableView</b> is implemented as a <b>QAbstractButton</b> and can be styled using the \"<b>QTableView QTableCornerButton::section</b>\" selector.<br><b style=\"color:#ff0000;\">Warning</b>: If you only set a background-color on a QTableCornerButton, the background may not appear unless you set the border property to some value. This is because, by default, the QTableCornerButton draws a native border which completely overlaps the background-color.<br>The color of the grid can be specified using the <b>gridline-color</b> property.<br>See <b>QAbsractScrollArea</b> to style scrollable backgrounds.");
    ///QTextEdit
    tip_txe = tr("The color and background of selected text is styled using <b>selection-color</b> and <b>selection-background-color</b> respectively.<br>See <b>QAbsractScrollArea</b> to style scrollable backgrounds.");
    ///QToolBar
    tip_tlb = tr("The <b>:top, :left, :right, :bottom</b> pseudo states depending on the area in which the tool bar is grouped.<br>The <b>:first, :last, :middle, :only-one</b> pseudo states indicator the position of the tool bar within a line group (See <b>QStyleOptionToolBar::positionWithinLine</b>).<br>The separator of a <b>QToolBar</b> is styled using the <b>::separator</b> subcontrol.<br>The handle (to move the toolbar) is styled using the <b>::handle</b> subcontrol.");
    ///QToolButton
    tip_tbt = tr("If the <b>QToolButton</b> has a menu, is <b>::menu-indicator</b> subcontrol can be used to style the indicator. By default, the menu-indicator is positioned at the bottom right of the Padding rectangle of the widget.<br>If the <b>QToolButton</b> is in <b>QToolButton::MenuButtonPopup</b> mode, the <b>::menu-button</b> subcontrol is used to draw the menu button. <b>::menu-arrow</b> subcontrol is used to draw the menu arrow inside the menu-button. By default, it is positioned in the center of the Contents rectangle of the menu-button subcontrol.<br>When the <b>QToolButton</b> displays arrows, the <b>::up-arrow, ::down-arrow, ::left-arrow and ::right-arrow</b> subcontrols are used.<br><b style=\"color:#ff0000;\"> Warning:</b> If you only set a background-color on a <b>QToolButton</b>, the background will not appear unless you set the border property to some value. This is because, by default, the <b>QToolButton</b> draws a native border which completely overlaps the background-color. For example,<br>QToolButton {<br> background-color: red;<br> border: none;<br> }");
    ///QToolBox
    tip_tbx = tr("The individual tabs can by styled using the <b>::tab</b> subcontrol. The tabs support the <b>:only-one, :first, :last, :middle, :previous-selected, :next-selected, :selected</b> pseudo states.");
    ///QToolTip
    tip_ttp = tr("The opacity property controls the opacity of the tooltip.<br> NB: QToolTip is a QFrame.");
    ///QTreeView, QTreeWidget
    tip_trv = tr("When <b>alternating row colors</b> is enabled, the alternating colors can be styled using the <b>alternate-background-color</b> property.<br>The color and background of the selected item is styled using <b>selection-color</b> and <b>selection-background-color</b> respectively.<br>The selection behavior is controlled by the <b>show-decoration-selected</b> property. The branches of the tree view can be styled using the <b>::branch</b> subcontrol. The ::branch Sub-control supports the <b>:open, :closed, :has-sibling</b> and <b>:has-children</b> pseudo states.<br>Use the <b>::item</b> subcontrol for more fine grained control over the items in the QTreeView. <br>See <b>QAbsractScrollArea</b> to style scrollable backgrounds.");
    ///QWidget
    tip_wid = tr("Supports only the <b>background, background-clip</b> and <b>background-origin</b> properties.<br> If you subclass from QWidget, you need read the <b>Qt Style Sheets Reference</b>");
}
//-----------------------------------------------------------------------
void MainWindow::showHelp(const QString &page)
{
    if (page.isEmpty() && !curHelpPage.isEmpty())
        helpView->load(curHelpPage);
    else if (QFile::exists(helpPath + "/qtwidgets/" + page))
        helpView->load("file://" + helpPath + "/qtwidgets/" + page);
    else
        helpView->setHtml(tr("<h1>Offline Help files is not installed :(</h1>"));

    ui->tabWidget->setCurrentIndex(1);
    helpView->setFocus(); //*/
}
//-----------------------------------------------------------------------
void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1)
        showHelp("");
}
//-----------------------------------------------------------------------
void MainWindow::setHelpPath()
{
    QString s = QFileDialog::getExistingDirectory(this, tr("Choose Qt Help Directory"),
                                                 helpPath,
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if (s.isEmpty())   return;
    helpPath = s;
    showHelp("stylesheet.html");

}
//-----------------------------------------------------------------------
void MainWindow::on_checkBoxHighlight_toggled(bool checked)
{
    checked? ui->checkBoxHighlight->setIcon(QIcon(":/res/cursor_h.png")) :
             ui->checkBoxHighlight->setIcon(QIcon(":/res/cursor.png"));
}
//-----------------------------------------------------------------------
void MainWindow::on_actionExportCreatedQSS_triggered()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Export QSS"), QString(),
                                              tr("QSS-files (*.qss);;TXT-files "
                                                 "(*.txt)"));

    if (!(fn.endsWith(".qss", Qt::CaseInsensitive)
          || fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".qss"; // default

    QTextDocumentWriter writer(fn, "TXT");
    bool success = writer.write(editorInternCss->document());
    if (success)
        consoleMsg(tr("Export QSS file:\n") + fn);

}
//-----------------------------------------------------------------------
void MainWindow::on_actionExportGeneralQSS_triggered()
{
    updateGeneralQssText();
    ui->tabWidget->setCurrentIndex(2);
    QString fn = QFileDialog::getSaveFileName(this, tr("Export QSS"), QString(),
                                              tr("QSS-files (*.qss);;TXT-files "
                                                 "(*.txt)"));

    if (!(fn.endsWith(".qss", Qt::CaseInsensitive)
          || fn.endsWith(".txt", Qt::CaseInsensitive)))
        fn += ".qss"; // default

    QTextDocumentWriter writer(fn, "TXT");
    bool success = writer.write(editorGeneral->document());
    if (success)
        consoleMsg(tr("Export QSS file:\n") + fn);
}
//-----------------------------------------------------------------------
