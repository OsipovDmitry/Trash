include(../include/build/build.pri)
TEMPLATE = lib

QT += core gui opengl

DEFINES += \
    CORE_LIBRARY

HEADERS += \
    ../include/core/coreglobal.h \
    ../include/core/forwarddecl.h \
    ../include/core/types.h \
    ../include/core/abstractcontroller.h \
    ../include/core/core.h \
    ../include/core/graphicscontroller.h \
    ../include/core/audiocontroller.h \
    ../include/core/abstractgame.h \
    src/renderwidget.h \
    src/coreprivate.h \
    src/renderer.h

SOURCES += \
    src/abstractcontroller.cpp \
    src/renderwidget.cpp \
    src/core.cpp \
    src/renderer.cpp \
    src/graphicscontroller.cpp \
    src/audiocontroller.cpp

