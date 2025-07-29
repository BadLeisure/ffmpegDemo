#-------------------------------------------------
#
# Project created by QtCreator 2020-04-19T16:40:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = zero_player
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += __STDC_CONSTANT_MACROS
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    qpainterdrawable.cpp \
    librtmp/amf.c \
    librtmp/hashswf.c \
    librtmp/log.c \
    librtmp/parseurl.c \
    librtmp/rtmp.c \
    dlog.cpp \
    looper.cpp \
    rtmpbase.cpp \
    naluloop.cpp \
    commonlooper.cpp \	
    mediabase.cpp \
     videooutsdl.cpp \
    audiooutsdl.cpp \
    avsync.cpp \
    framequeue.cpp \
     rtmpplayer.cpp \
    aacdecoder.cpp \
    h264decoder.cpp \
    audiodecodeloop.cpp \
    pullwork.cpp \
    videooutputloop.cpp \
    avtimebase.cpp \ 
    packetqueue.cpp \
    imagescale.cpp \
    videodecodeloop.cpp \
    sonic.cpp
win32 {
INCLUDEPATH += $$PWD/ffmpeg-4.2.1-win32-dev/include
LIBS += $$PWD/ffmpeg-4.2.1-win32-dev/lib/avformat.lib   \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/avcodec.lib    \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/avdevice.lib   \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/avfilter.lib   \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/avutil.lib     \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/postproc.lib   \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/swresample.lib \
        $$PWD/ffmpeg-4.2.1-win32-dev/lib/swscale.lib
LIBS += D:\Qt\Qt5.10.1\Tools\mingw530_32\i686-w64-mingw32\lib\libws2_32.a
LIBS += "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.19041.0\um\x86\WinMM.Lib"
INCLUDEPATH += $$PWD/SDL2/include
LIBS += $$PWD/SDL2/lib/x86/SDL2.lib
}

HEADERS += \
librtmp/amf.h \
    librtmp/bytes.h \
    librtmp/dh.h \
    librtmp/dhgroups.h \
    librtmp/handshake.h \
    librtmp/http.h \
    librtmp/log.h \
    librtmp/rtmp.h \
    librtmp/rtmp_sys.h \
    dlog.h \
    looper.h \
    semaphore.h \
    rtmpbase.h \
    mediabase.h \
    naluloop.h \
    commonlooper.h \
    timeutil.h \
    rtmpplayer.h \
    videooutsdl.h \
    audiooutsdl.h \
    avsync.h \
    avtimebase.h\
    aacdecoder.h \
    audiodecodeloop.h \
    pullwork.h \
    videodecodeloop.h \
    videooutputloop.h \
    framequeue.h \ 
    packetqueue.h \
    imagescale.h \
    mainwindow.h \
    qpainterdrawable.h
FORMS += \
        mainwindow.ui \
    qpainterdrawable.ui
