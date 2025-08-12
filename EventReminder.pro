QT += core gui widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    addeventdialog.cpp

HEADERS += \
    mainwindow.h \
    addeventdialog.h

FORMS += \
    mainwindow.ui \
    addeventdialog.ui

# For Linux autostart path expansion
DEFINES += QT_NO_CAST_FROM_ASCII
