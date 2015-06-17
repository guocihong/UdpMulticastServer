
#-------------------------------------------------
#
# Project created by QtCreator 2015-04-17T14:26:31
#
#-------------------------------------------------

QT       += core gui sql xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UdpMulticastServer
TEMPLATE = app


SOURCES += main.cpp\
        udpmulticastserver.cpp \
    shuakajiconfig.cpp \
    deviceupgrade.cpp \
    sendfileclient.cpp \
    iconhelper.cpp \
    jiayouzhanconfig.cpp

HEADERS  += udpmulticastserver.h \
    CommonSetting.h \
    shuakajiconfig.h \
    deviceupgrade.h \
    sendfileclient.h \
    myhelper.h \
    iconhelper.h \
    jiayouzhanconfig.h

FORMS    += udpmulticastserver.ui \
    shuakajiconfig.ui \
    deviceupgrade.ui \
    jiayouzhanconfig.ui

win32{
LIBS += -lWs2_32
}

RC_FILE=main.rc

DESTDIR=bin
MOC_DIR=temp/moc
RCC_DIR=temp/rcc
UI_DIR=temp/ui
OBJECTS_DIR=temp/obj
