include(../include/build/build.pri)
TEMPLATE = lib

QT += core

DEFINES += \
    GAME_LIBRARY

HEADERS += \
    ../include/game/game.h \
    ../include/game/gameglobal.h

SOURCES += \
    src/game.cpp

LIBS += \
    -lcore

RESOURCES +=

