QT *= network

if(! include($$PWD/../networkprefix/networkprefix.pri) ) {
    message("Unable to load networkprefix.pri")
}

INCLUDEPATH += $$PWD

SOURCES += \
    networkprefixset.cpp

HEADERS += \
    networkprefixset.h
