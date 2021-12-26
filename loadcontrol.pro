TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    loadcontrol.c \
    nettool.c

LIBS += -L$$PWD/ -lpthread

unix:!macx: LIBS += -L$$PWD/../../../../usr/local/arm/tslib/lib/ -lts

INCLUDEPATH += $$PWD/../../../../usr/local/arm/tslib/include
DEPENDPATH += $$PWD/../../../../usr/local/arm/tslib/include

HEADERS += \
    net.h \
    nettool.h \
    loadcontrol.h

INSTALLS += target
target.path = /home/forlinx
