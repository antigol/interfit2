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
    function.cc \
    mainwindow.cc \
    metropolis.cc

FORMS += mainwindow.ui

HEADERS += function.hh \
    mainwindow.hh \
    metropolis.hh \
    thinfilm/thinfilm.hh
