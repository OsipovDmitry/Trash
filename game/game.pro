include(../include/build/build.pri)
TEMPLATE = lib

QT += core

DEFINES += \
    GAME_LIBRARY

HEADERS += \
    ../include/game/game.h \
    ../include/game/gameglobal.h \
    src/object.h \
    src/scene.h \
    src/gameprivate.h \
    src/floor.h \
    src/person.h \
    src/teapot.h \
    src/waypoint.h

SOURCES += \
    src/game.cpp \
    src/scene.cpp \
    src/gameprivate.cpp \
    src/floor.cpp \
    src/person.cpp \
    src/teapot.cpp \
    src/waypoint.cpp

LIBS += \
    -lcore

RESOURCES +=

