#-------------------------------------------------
#
# Project created by QtCreator 2017-07-15T12:41:04
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += core websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PortDebug
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    serialport.cpp \
    socketserver.cpp

HEADERS  += widget.h \
    serialport.h \
    socketserver.h

FORMS    += widget.ui

RESOURCES += \
    resources.qrc


RC_ICONS += "Icon/serialport.ico"

