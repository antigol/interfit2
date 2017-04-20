QT += gui
QT += widgets
QT += multimedia

CONFIG += c++11

TARGET = interfit

DEFINES += QT_DEPRECATED_WARNINGS

DEPENDPATH += . lockin2
INCLUDEPATH += . lockin2

include(lockin2/lockin.pri)

SOURCES += main.cc \
    mainwindow.cc \
    metropolis.cc \
    model.cc

FORMS += mainwindow.ui

HEADERS += \
    mainwindow.hh \
    metropolis.hh \
    thinfilm/thinfilm.hh \
    model.hh
