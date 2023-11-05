QT -= gui
QT += core network

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = rmoserver

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.

HEADERS += \
    rmoserver.h\
    radarparameters.h

SOURCES += \
        main.cpp \
        rmoserver.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

