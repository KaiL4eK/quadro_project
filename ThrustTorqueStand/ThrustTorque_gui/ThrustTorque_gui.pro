#-------------------------------------------------
#
# Project created by QtCreator 2016-05-07T20:45:28
#
#-------------------------------------------------

QT       += core gui \
            serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ThrustTorque_gui
TEMPLATE = app

QWT_PATH = /usr/local/qwt-6.1.2

include ( $${QWT_PATH}/features/qwt.prf )

INCLUDEPATH += $${QWT_PATH}/include \
               ../

LIBS += -L $${QWT_PATH}/lib \
        -l qwt

SOURCES += main.cpp\
        mainwindow.cpp \
        serialLink.cpp \
        qwtPlotManager.cpp

HEADERS  += mainwindow.h \
        serialLink.h \
        qwtPlotManager.h

CONFIG += c++11
