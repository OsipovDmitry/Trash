include(../include/build/build.pri)
TEMPLATE = lib

QT += core

DEFINES += \
    TEETH_LIBRARY

HEADERS += \
    ../include/teeth/teeth.h \
    ../include/teeth/teethglobal.h

SOURCES += \
    src/teeth.cpp

LIBS += \
    -lcore

RESOURCES +=

