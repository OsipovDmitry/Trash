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
    ../include/core/node.h \
    ../include/core/trianglenode.h \
    src/renderwidget.h \
    src/coreprivate.h \
    src/renderer.h \
    src/resourcestorage.h \
    src/nodeprivate.h \
    src/graphicscontrollerprivate.h

SOURCES += \
    src/abstractcontroller.cpp \
    src/renderwidget.cpp \
    src/core.cpp \
    src/renderer.cpp \
    src/graphicscontroller.cpp \
    src/audiocontroller.cpp \
    src/resourcestorage.cpp \
    src/node.cpp \
    src/nodeprivate.cpp \
    src/graphicscontrollerprivate.cpp \
    src/trianglenode.cpp

RESOURCES += \
    core.qrc

