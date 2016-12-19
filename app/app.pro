TARGET = mixer
QT = quickcontrols2

HEADERS += \
    pacontrolmodel.h \
    pac.h

SOURCES = main.cpp \
    pacontrolmodel.cpp \
    pac.c

LIBS = -lpulse

RESOURCES += \
    Mixer.qrc

include(app.pri)
