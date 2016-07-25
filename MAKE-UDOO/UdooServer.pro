QT += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = UdooServer
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1y

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

FORMS   +=

INCLUDEPATH += $$PWD/../deps/libwatermark
DEPENDPATH += $$PWD/../deps/libwatermark

INCLUDEPATH += /usr/include/oscpack/ /home/pi/boiteselec-interfaceqt/deps/rtaudio-4.1.2/


LIBS+= -lgomp -lsndfile
LIBS+= -lrtaudio
LIBS+= -lasound
LIBS+= -loscpack
LIBS+= -lwiringPi

LIBS+= -L$$PWD/../deps/karchive-5.17.0/src -L$$PWD/../deps/karchive-5.17.0/build/src -lKF5Archive
INCLUDEPATH += $$PWD/../deps/karchive-5.17.0/src $$PWD/../deps/karchive-5.17.0/build/src
DEPENDPATH += $$PWD/../deps/karchive-5.17.0/src $$PWD/../deps/karchive-5.17.0/build/src
PRE_TARGETDEPS += $$PWD/../deps/karchive-5.17.0/build/src/libKF5Archive.so.5

RESOURCES += 

OTHER_FILES += 


#### Libraries ####
  ##  Oscpack  ##
# unix:!macx: LIBS += -L$$PWD/../../../git/oscpack/build/ -loscpack

# INCLUDEPATH += $$PWD/../../../git/oscpack/src
# DEPENDPATH += $$PWD/../../../git/oscpack/src

#unix:!macx: PRE_TARGETDEPS += $$PWD/../../../git/oscpack/build/liboscpack.a
