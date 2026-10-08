QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = TetrisGame
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    tetrisengine.cpp \
    tetriscanvas.cpp \
    previewcanvas.cpp

HEADERS += \
    mainwindow.h \
    tetrisconstants.h \
    tetrisengine.h \
    tetriscanvas.h \
    previewcanvas.h

FORMS += \
    mainwindow.ui

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
