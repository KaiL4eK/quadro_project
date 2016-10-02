#-------------------------------------------------
#
# Project created by QtCreator 2016-05-10T16:28:36
#
#-------------------------------------------------

QT       += core gui \
            serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IMUSimulator
TEMPLATE = app

QWT_PATH = /usr/local/qwt-6.1.2

INCLUDEPATH += $${QWT_PATH}/include

LIBS += -L $${QWT_PATH}/lib \
        -l qwt

SOURCES += main.cpp\
        mainwindow.cpp \
        seriallinker.cpp \
        datastreamer.cpp

HEADERS  += mainwindow.h \
            seriallinker.h \
            datastreamer.h \
            imudata.h

FORMS    += mainwindow.ui

CONFIG += c++11

QMAKE_CFLAGS += -fpermissive
QMAKE_CXXFLAGS += -fpermissive
QMAKE_LFLAGS += -fpermissive
