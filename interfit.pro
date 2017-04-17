QT += gui
QT += widgets
QT += multimedia

CONFIG += c++11

TARGET = interfit

DEFINES += QT_DEPRECATED_WARNINGS

DEPENDPATH += . lockin2
INCLUDEPATH += . lockin2

SOURCES += \
    main.cc \
    lockin2/src/lockin_gui.cc \
    lockin2/src/lockin.cc \
    lockin2/src/fifo.cc \
    lockin2/src/audioutils.cc \
    lockin2/xygraph/xyview.cc \
    lockin2/xygraph/xyscene.cc \
    lockin2/xygraph/xygraph.cc \
    function.cc \
    mainwindow.cc \
    metropolis.cc

FORMS += \
    lockin2/src/lockin_gui.ui \
    mainwindow.ui

HEADERS += \
    lockin2/src/lockin_gui.hh \
    lockin2/src/lockin.hh \
    lockin2/src/fifo.hh \
    lockin2/src/audioutils.hh \
    lockin2/xygraph/xyview.hh \
    lockin2/xygraph/xyscene.hh \
    lockin2/xygraph/xygraph.hh \
    function.hh \
    mainwindow.hh \
    metropolis.hh \
    thinfilm/thinfilm.hh
