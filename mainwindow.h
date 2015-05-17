#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QSpinBox>
#include <QCompleter>
#include <QStringList>
#include <QWebView>
#include <QMovie>
#include <QLabel>
#include "qsssheet.h"
#include "colorwidget.h"
#include "combowidgets.h"
#include "qsshighlighter.h"
#include "qsscodeeditor.h"

class QWebView;
QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *e);
    bool eventFilter( QObject *obj, QEvent *evt );

signals:

protected slots:

    void adjustLocation();
    void changeLocation();
    void adjustTitle();
    void setProgress(int p);
    void finishLoading(bool);
    void statusBarMsg(QString s);

private slots:
    void on_actionNewTemplate_triggered()  { clearTemplate();    }
    void on_actionOpenTemplate_triggered() { openTemplateFile(); }
    void on_actionSaveTemplate_triggered() { templateFileSave(); }
    void on_actionNewExtQssFile_triggered(){ qssFileNew();  }
    void on_actionOpenQssFile_triggered() { openQssFile(); }
    void on_actionSaveExternalQssFile_triggered() { qssFileSave(); }
    void applyStyleSheet();
    void setQStyle(const QString &style) { qApp->setStyle(style); }
    void updateQssText();
    void updateGeneralQssText();
    void openRecentTemplateFile();
    void openRecentQssFile();
    void setHelpPath();
    void on_groupBoxApplyQss_clicked()   { applyStyleSheet(); }
    void on_radioBtnCreatedQss_clicked() { applyStyleSheet(); }
    void on_radioBtnLoadedQss_clicked()  { applyStyleSheet(); }
    void on_radioBtnBothQss_clicked()    { applyStyleSheet(); }
    void on_actionOfflineHelp_triggered(){ showHelp("stylesheet.html"); }
    void on_actionExamples_triggered(){ showHelp("stylesheet-examples.html"); }
    void on_actionSaveTemplateAs_triggered() { templateFileSaveAs(); }
    void on_actionSaveExternalQssAs_triggered() { qssFileSaveAs(); }
    void on_checkBoxHighlight_toggled(bool checked);
    void on_actionOpenUIFile_triggered();
    void on_actionLoadUifile_triggered() {loadUiFile(":/res/default.ui");}
    void on_actionExportCreatedQSS_triggered();
    void on_actionExportGeneralQSS_triggered();
    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;
    QComboBox *styleCombo;
    void writeSettings();
    void readSettings();
    void consoleMsg(const QString &msg);
    bool saveTemlateFile(const QString &fileName);
    void openQssFile();
    void openTemplateFile();
    bool loadQssFile(const QString &fileName);
    void clearTemplate();
    void updateRecentTemplateFileActions();
    void updateRecentQssFileActions();
    QString strippedName(const QString &fullFileName);
    void loadTemplCaption(const QString &line, bool closed);
    void loadTemplParam(const QString &line, int strNum);
    void addBoxWidgets();
    void addFontWidgets();
    void createSpinWidget(const QString &sName, const QString &sParam, int strNum);
    void createColorWidget(const QString &sName, const QString &sType, const QString &sParam, int strNum);
    void setupTips();
    bool maybeTemplateSave();
    bool maybeQssSave();
    bool qssFileSave();
    bool qssFileSaveAs();
    bool templateFileSave();
    bool templateFileSaveAs();
    void setCurrentQssFileName(const QString &fileName);
    void setCurrentTemplateFileName(const QString &fileName);
    void qssFileNew();
    void loadTemplate(const QString &file);
    void loadUiFile(const QString &fileName);
    void showHelp(const QString &page);
    enum { MaxRecentTemplateFiles = 8, MaxRecentQssFiles = 8 };
    QAction *recentTemplateFileActions[MaxRecentTemplateFiles];
    QAction *recentQssFileActions[MaxRecentQssFiles];
    QssSheet *qsssheet;
    QssHighlighter *qsshighlighter;
    QssHighlighter *qsshighlighterBase;
    QssHighlighter *qsshighlighterGen;
    QAbstractItemModel *modelFromFile(const QString& fileName);
    QCompleter *completer;
    QssCodeEditor *editorInternCss;
    QssCodeEditor *editorExternCss;
    QssCodeEditor *editorGeneral;
    QssCodeEditor *editorDeep;
    QString tip_asa, tip_aiv, tip_abt, tip_abs, tip_chb, tip_clv, tip_cbb, tip_spb,
            tip_dlg, tip_dbb, tip_dkw, tip_frm, tip_grb, tip_hdv, tip_lbl, tip_lne,
            tip_lsv, tip_mwd, tip_mnu, tip_mnb, tip_mbx, tip_prb, tip_pbt, tip_cbt,
            tip_scr, tip_sgr, tip_sld, tip_spl, tip_stb, tip_tbr, tip_tbw, tip_tbv,
            tip_txe, tip_tlb, tip_tbt, tip_tbx, tip_ttp, tip_trv, tip_wid; /* */
    QRubberBand *rubberBand;
    QStringList recentTemplateFiles;
    QStringList recentQssFiles;
    QMenu *recentTemplateMenu;
    QMenu *recentQssMenu;
    QString curTemplateFile;
    QString curQssFile;
    QString curUiFile;
    QString helpPath;
    QString curHelpPage;
    bool bTemplateChahged;
    QWebView *helpView;
    QToolBar *helpToolBar;
    QLineEdit *locationEdit;
    int progress;
    QUrl url;
    QLabel *labelHelpTitle;
    QLabel *labelHelpStatus;
    bool bWait;
};
//-----------------------------------------------------------------------
class UrlLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit UrlLineEdit(QWidget *parent = 0);

signals:
    void atas();

public slots:
    void setStatustip(QString s);
    void setValue(const QString &value);

private:
    QLineEdit *urlLineEdit;

};
#endif // MAINWINDOW_H
