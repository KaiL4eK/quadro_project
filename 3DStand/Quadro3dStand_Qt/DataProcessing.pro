#-------------------------------------------------
#
# Project created by QtCreator 2016-05-07T20:45:28
#
#-------------------------------------------------

QT       += core gui \
            serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataProcessing
TEMPLATE = app

INCLUDEPATH +=  $(HOME)/qwt/include \
                $(HOME)/Qt/qwt-6.1.4/include \
                ../../common

LIBS += -L$(HOME)/Qt/qwt-6.1.4/lib -lqwt

SOURCES += main.cpp\
        mainwindow.cpp \
        qwtPlotter.cpp \
        serialLink.cpp

HEADERS  += mainwindow.h \
        qwtPlotter.h \
        serialLink.h \
    ../../common/serial_protocol.h

CONFIG += c++11
