QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = barnes-hut-sim
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++0x

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    quadtree.cpp \
    calcthread.cpp

HEADERS += \
    mainwindow.h \
    quadtree.h \
    calcthread.h

RESOURCES += \
    main.qrc

OTHER_FILES += \
    main.frag \
    main.vert
