TEMPLATE = app
#CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

LIBS += -L$$PWD/lib
LIBS += -liconv

INCLUDEPATH += include

SOURCES += main.c
