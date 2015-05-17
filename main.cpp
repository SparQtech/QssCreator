#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;

#ifndef HAVE_QT5
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));
#endif

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator Translator;
   if (Translator.load("qsscreator_" + QLocale::system().name(),
            qApp->applicationDirPath()))
        app.installTranslator(&Translator);

    w.show();

    return app.exec();
}
