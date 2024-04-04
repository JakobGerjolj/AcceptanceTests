QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
QT += serialport
TEMPLATE = app

SOURCES +=  tst_test.cpp

unix:!macx: LIBS += -L$$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug/ -ltestBenchLibrary

INCLUDEPATH += $$PWD/../build-testBenchLibrary-Desktop_Qt_6_5_3_GCC_64bit-Debug
DEPENDPATH += $$PWD/../build-testBenchLibrary-Desktop_Qt_6_5_3_GCC_64bit-Debug

unix:!macx: PRE_TARGETDEPS += $$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug/libtestBenchLibrary.a
