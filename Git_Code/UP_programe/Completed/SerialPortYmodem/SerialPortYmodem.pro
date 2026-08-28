#-------------------------------------------------
#
# Project created by QtCreator 2018-01-20T23:00:46
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app
TARGET = $$qtLibraryTarget(BootApp)
# 固定输出到工程源码的上级目录（UP_programe\Completed），不随 build 目录漂移
DESTDIR = $${PWD}/..

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

##防止中文乱码
#msvc:QMAKE_CXXFLAGS += -execution-charset:utf-8
#msvc:QMAKE_CXXFLAGS += -source-charset:utf-8
#QMAKE_CXXFLAGS_WARN_ON += -wd4819



SOURCES += main.cpp\
    IMUBoot/imubootloader.cpp \
    NavBoot/navbootloader.cpp \
        widget.cpp \
    YmodemFileReceive.cpp \
    Ymodem.cpp \
    YmodemFileTransmit.cpp

HEADERS  += widget.h \
    IMUBoot/imubootloader.h \
    NavBoot/navbootloader.h \
    Ymodem.h \
    YmodemFileReceive.h \
    YmodemFileTransmit.h

FORMS    += widget.ui

RC_ICONS = SerialPortYmodem.ico


