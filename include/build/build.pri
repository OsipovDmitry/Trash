CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../../compile/Debug
} else {
    DESTDIR = $$PWD/../../compile/Release
}
OBJECTS_DIR = $$DESTDIR/.obj/$$TARGET
MOC_DIR = $$DESTDIR/.moc/$$TARGET
RCC_DIR = $$DESTDIR/.qrc/$$TARGET
UI_DIR = $$DESTDIR/.ui/$$TARGET

INCLUDEPATH += ../include

LIBS += -L$$DESTDIR

QT -= core gui
CONFIG += c++14