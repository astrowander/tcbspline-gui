#-------------------------------------------------
#
# Project created by QtCreator 2016-01-22T20:33:50
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++0x
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = tcbspline
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h

FORMS    += mainwindow.ui
