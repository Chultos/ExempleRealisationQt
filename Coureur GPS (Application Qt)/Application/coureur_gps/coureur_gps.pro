#-------------------------------------------------
#
# Project created by QtCreator 2013-11-29T11:37:58
#
#-------------------------------------------------

QT       += core gui
QT      += network
QT      += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test_client_tcp
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    QRC.qrc
