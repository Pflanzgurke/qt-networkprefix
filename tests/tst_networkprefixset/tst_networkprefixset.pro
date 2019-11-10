QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

if(! include(../../networkprefixset/networkprefixset.pri) ) {
    message("Unable to load networkprefixset.pri")
}

SOURCES +=  tst_networkprefixset.cpp

RESOURCES += \
    tst_networkprefixset.qrc

DISTFILES += \
    tst_input_correct.txt \
    tst_input_with_duplicates.txt \
    tst_input_with_errors.txt
