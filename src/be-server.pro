QT += core serialport
QT -= gui

greaterThan(QT_MAJOR_VERSION, 4): QT += serialport

TARGET = be-server
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1y -Wall -Wextra

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast -march=armv6 -flto -fopenmp
QMAKE_LFLAGS_RELEASE -= -Wl,-O1
QMAKE_LFLAGS_RELEASE += -Wl,-O3 -Wl,-flto

QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG += -O0 -Wno-unknown-pragmas

SOURCES += main.cpp\
	Server.cpp \
        Track.cpp \
	PlayThread.cpp \
	SaveManager.cpp \
        SerialManager.cpp

HEADERS += Server.h \
	Track.h \
	PlayThread.h \
        SongData.h \
	SaveManager.h \
        SerialManager.h

INCLUDEPATH += 	$$PWD/../deps/libwatermark
DEPENDPATH += $$PWD/../deps/libwatermark

INCLUDEPATH += /usr/include/oscpack

# KArchive
INCLUDEPATH += /usr/local/include/KF5/KArchive
DEPENDPATH += /usr/local/lib/arm-linux-gnueabihf
PRE_TARGETDEPS += /usr/local/lib/arm-linux-gnueabihf/libKF5Archive.so.5


LIBS+= -lgomp -lsndfile
LIBS+= -lrtaudio
LIBS+= -lasound
LIBS+= -loscpack
LIBS+= -lwiringPi
LIBS+= -lKF5Archive

RESOURCES += 

OTHER_FILES += 
