#-------------------------------------------------
#
# Project created by QtCreator 2015-07-17T17:13:44
#
#-------------------------------------------------

QT       += core gui
QT += phonon
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MusicPlayer
TEMPLATE = app
RC_FILE = APPIcon.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    canceldottedline.cpp \
    musicplaylist.cpp \
    lyrics.cpp

HEADERS  += mainwindow.h \
    canceldottedline.h \
    musicplaylist.h \
    lyrics.h

RESOURCES += \
    image.qrc \
    style.qrc
