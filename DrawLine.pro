QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    my_label.cpp

HEADERS += \
    mainwindow.h \
    my_label.h

FORMS += \
    mainwindow.ui

# Default rules for deployment
unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target