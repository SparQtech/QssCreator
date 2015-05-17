#-------------------------------------------------
#
# Project created by Spark (Oleg Belyavsky)
# soft.spark@gmail.com
#-------------------------------------------------

QT       += core gui webkit webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  DEFINES += HAVE_QT5
}
greaterThan(QT_VERSION, 4.5) {
  QT +=  uitools
  DEFINES += HAVE_QT4_5
}

#equals(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 8) {

TARGET = qsscreator
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        qsssheet.cpp \
        qsshighlighter.cpp \
        qsscodeeditor.cpp \
        colorwidget.cpp \
        combowidgets.cpp \
        gradientdialog.cpp \
        spinwidgets.cpp

HEADERS += mainwindow.h\
        qsssheet.h \
        colorwidget.h \
        qsshighlighter.h \
        qsscodeeditor.h \
        combowidgets.h \
        gradientdialog.h \
        spinwidgets.h

FORMS    += mainwindow.ui \
         gradientdialog.ui

TRANSLATIONS = qsscreator_ru.ts

RESOURCES = qsscreator.qrc

win32 {
    RC_FILE = qsscreator.rc
}

OTHER_FILES +=
