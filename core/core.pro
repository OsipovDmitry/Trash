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
    src/image.h \
    src/texture.inl \
    src/drawablenodeprivate.h \
    src/textnodeprivate.h \
    src/autotransformnodeprivate.h \
    src/utils.h \
    src/typesprivate.h \
    src/scenerootnodeprivate.h \
    src/settingsprivate.h \
    src/nodeupdatevisitor.h \
    src/noderendershadowmapvisitor.h \
    src/noderendervisitor.h \
    src/nodepickvisitor.h \
    src/particlesystemnodeprivate.h

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
    src/scene.cpp \
    src/light.cpp \
    src/lightprivate.cpp \
    src/sceneprivate.cpp \
    src/image.cpp \
    src/teapotnode.cpp \
    src/drawablenode.cpp \
    src/drawablenodeprivate.cpp \
    src/textnode.cpp \
    src/textnodeprivate.cpp \
    src/autotransformnode.cpp \
    src/autotransformnodeprivate.cpp \
    src/utils.cpp \
    src/scenerootnode.cpp \
    src/scenerootnodeprivate.cpp \
    src/settings.cpp \
    src/settingsprivate.cpp \
    src/primitivenode.cpp \
    src/nodeintersectionvisitor.cpp \
    src/particlesystemnode.cpp \
    src/particlesystemnodeprivate.cpp

LIBS += \
#    -lassimp-vc140-mt
    -lassimp

RESOURCES += \
    core.qrc

