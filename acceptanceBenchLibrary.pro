QT += testlib
QT -= gui
QT += serialbus

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
QT += serialport
TEMPLATE = app

SOURCES +=  tst_test.cpp

INCLUDEPATH += /home/jakob/rpi-sysroot/usr/include/

LIBS += -L"/home/jakob/rpi-sysroot/usr/lib" -lwiringPi
LIBS += -L$$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug/ -ltestBenchLibrary


INCLUDEPATH += $$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug
DEPENDPATH += $$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug

unix:!macx: PRE_TARGETDEPS += $$PWD/../build-testBenchLibrary-Qt_6_5_3_qt_raspi-Debug/libtestBenchLibrary.a
