#-------------------------------------------------
#
# Project created by QtCreator 2012-01-14T21:05:21
#
#-------------------------------------------------

QT       += core phonon xml network

TARGET = local-plugin-for-hathor
TEMPLATE = lib
CONFIG += plugin oauth

#DESTDIR = /usr/lib/hathor/plugins
win32:LIBS += -llastfm0
debug:win32:LIBS += "../libhathor/debug/hathor.lib"
release:win32:LIBS += "../libhathor/release/hathor.lib"

SOURCES += hlocalprovider.cpp

HEADERS += hlocalprovider.h

INCLUDEPATH+="../libhathor"
