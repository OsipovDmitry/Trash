include(../include/build/build.pri)
TEMPLATE = lib

QT += core gui opengl

DEFINES += \
    CORE_LIBRARY

HEADERS += \
    ../include/core/renderwidget.h \
    ../include/core/coreglobal.h

SOURCES += \
    src/renderwidget.cpp
