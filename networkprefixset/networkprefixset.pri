QT *= network

if(! include($$PWD/../networkprefix/networkprefix.pri) ) {
    message("Unable to load networkprefix.pri")
}

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/networkprefixset.cpp

HEADERS += \
    $$PWD/networkprefixset.h
