QT += testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

if(! include(../../networkprefix/networkprefix.pri) ) {
    message("Unable to load networkprefix.pri")
}

SOURCES +=  tst_networkprefix.cpp
