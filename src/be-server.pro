QT += core gui serialport
CONFIG += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = be-server
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1y

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast -march=native -flto -fopenmp
QMAKE_LFLAGS_RELEASE -= -Wl,-O1
QMAKE_LFLAGS_RELEASE += -Wl,-O3 -Wl,-flto

QMAKE_CXXFLAGS_DEBUG -= -O1
QMAKE_CXXFLAGS_DEBUG -= -O2
QMAKE_CXXFLAGS_DEBUG += -O0 -Wno-unknown-pragmas

SOURCES += main.cpp\
        Track.cpp \
        PlayThread.cpp \
	SaveManager.cpp \
        SerialManager.cpp \
    Server.cpp

HEADERS += \
        Track.h \
        PlayThread.h \
        SongData.h \
	SaveManager.h \
        SerialManager.h \
    Server.h

INCLUDEPATH += $$PWD/../../libaudiotool/src/libwatermark
DEPENDPATH += $$PWD/../../libaudiotool/src/libwatermark

LIBS+= -lgomp -lsndfile
#LIBS+= -lportaudiocpp -lportaudio
LIBS+= -lrtaudio
LIBS+= -lasound
#LIBS+= -lgcov -lavcodec -lavformat -lavutil


INCLUDEPATH += /usr/local/include/KF5/KArchive  /usr/include/KF5/KArchive
LIBS += -lKF5Archive

#### Libraries ####
  ##  Oscpack  ##
linux-g++ {
unix:!macx: LIBS += -L$$PWD/../../deps/linux/oscpack/ -loscpack

INCLUDEPATH += $$PWD/../../deps/src/oscpack
DEPENDPATH += $$PWD/../../deps/src/oscpack

unix:!macx: PRE_TARGETDEPS += $$PWD/../../deps/linux/oscpack/liboscpack.a
}
