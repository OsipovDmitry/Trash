include(../include/build/build.pri)
TEMPLATE = lib

QT += core gui opengl

DEFINES += \
    CORE_LIBRARY

HEADERS += \
    src/hdrloader/hdrloader.h \
    src/rapidjson/*.h \
    ../include/utils/*.h \
    ../include/core/*.h \
    src/renderwidget.h \
    src/coreprivate.h \
    src/cameraprivate.h \
    src/renderer.h \
    src/resourcestorage.h \
    src/model.inl \
    src/nodeprivate.h \
    src/graphicscontrollerprivate.h \
    src/abstractcontrollerprivate.h \
    src/modelnodeprivate.h \
    src/drawables.h \
    src/resources.h \
    src/importexport.h \
    src/sceneprivate.h \
    src/lightprivate.h \
    src/cameraprivate.h \
    src/image.h \
    src/texture.inl

SOURCES += \
    src/hdrloader/hdrloader.cpp \
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
    src/coreprivate.cpp \
    src/modelnode.cpp \
    src/modelnodeprivate.cpp \
    src/drawables.cpp \
    src/importexport.cpp \
    src/camera.cpp \
    src/cameraprivate.cpp \
    src/scene.cpp \
    src/light.cpp \
    src/lightprivate.cpp \
    src/sceneprivate.cpp \
    src/image.cpp \
    src/teapotnode.cpp

LIBS += \
    -lassimp-vc140-mt
#    -lassimp

RESOURCES += \
    core.qrc

