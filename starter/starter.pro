include(../include/build/build.pri)
TEMPLATE = app

QT += widgets

SOURCES += \
    src/main.cpp

LIBS += \
    -lcore \
    -lgame
