TEMPLATE = app
CONFIG += console
#c++11
CONFIG -= app_bundle
CONFIG -= qt
#DEFINES += DENABLE_PRECOMPILED_HEADERS=OFF


#DEFINES += _WIN32
SOURCES += main_publish.cpp\
    librtmp/amf.c \
    librtmp/hashswf.c \
    librtmp/log.c \
    librtmp/parseurl.c \
    librtmp/rtmp.c \
    rtmp_publish.cpp \
    aacencoder.cpp \
    audioresample.cpp \
    dlog.cpp \
    rtmppusher.cpp \
    looper.cpp \
    h264encoder.cpp \
    rtmpbase.cpp \
    naluloop.cpp \
    pushwork.cpp \
    audiocapturer.cpp \
    commonlooper.cpp \
    mediabase.cpp \
    aacrtmppackager.cpp \
    videocapturer.cpp \
    videooutsdl.cpp \
    avtimebase.cpp

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
#LIBS += D:\Qt\Qt5.10.1\Tools\mingw530_32\i686-w64-mingw32\lib\libws2_32.a
LIBS += C:\Qt\Qt5.14.1\Tools\mingw730_32\i686-w64-mingw32\lib\libws2_32.a

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
    aacencoder.h \
    audio.h \
    audioresample.h \
    dlog.h \
    rtmppusher.h \
    looper.h \
    semaphore.h \
    h264encoder.h \
    rtmpbase.h \
    mediabase.h \
    naluloop.h \
    pushwork.h \
    audiocapturer.h \
    commonlooper.h \
    timeutil.h \
    rtmppackager.h \
    aacrtmppackager.h \
    videocapturer.h \
    rtmpplayer.h \
    videooutsdl.h \
    audiooutsdl.h \
    ringbuffer.h \
    avsync.h \
    avtimebase.h
