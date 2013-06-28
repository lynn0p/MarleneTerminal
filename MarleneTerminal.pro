#-------------------------------------------------
#
# Project created by QtCreator 2013-06-23T14:53:58
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MarleneTerminal
TEMPLATE = app


SOURCES += main.cpp \
    mainwindow.cpp \
    uicardnetbroker.cpp

HEADERS  += mainwindow.h \
    uicardnetbroker.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$PWD/../MarleneLib
debug {
LIBS += -L$$PWD/../build-MarleneLib-Debug/debug -lMarleneLib
}
release {
LIBS += -L$$PWD/../build-MarleneLib-Release/release -lMarleneLib
}

win32 {
INCLUDEPATH += C:\BasicCardPro\Api\H
LIBS += -LC:\BasicCardPro\Api\Lib\MS_new -lzccri -lzcbci
LIBS += WS2_32.lib
}
