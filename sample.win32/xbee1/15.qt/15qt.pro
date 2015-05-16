#-------------------------------------------------
#
# Project created by QtCreator 2013-07-14T12:43:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 15qt
TEMPLATE = app

SOURCES += main.cpp\
        window.cpp\
        xbeeqt.cpp

HEADERS  += window.h\
        xbeeqt.h

FORMS    += window.ui

unix: LIBS += -lxbeep -lxbee

win32: SOURCES += $$PWD/../../proj/libxbee3-win32/xbeep.cpp
win32: LIBS += -L$$PWD/../../proj/libxbee3-win32/lib/ -llibxbee3
win32: INCLUDEPATH += $$PWD/../../proj/libxbee3-win32
win32: DEPENDPATH += $$PWD/../../proj/libxbee3-win32
