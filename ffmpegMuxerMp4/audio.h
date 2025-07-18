#ifndef AUDIOENCODER_H
#define AUDIOENCODER_H
#include <vector>
extern "C"{

    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}


class AudioEncode{

public:
    AudioEncode();
    ~AudioEncode();
    int initAAC(int channels,int sampleRate,int bit_rate);
    void DeInit();  // 释放资源
    int GetFrameSize(); // 获取一帧数据 每个通道需要多少个样本点
    int GetSampleFormat();  // 编码器需要的采样格式
    AVCodecContext *GetCodecContext();
    int GetChannels();
    int GetSampleRate();
    AVPacket* encode(AVFrame *frame, int stream_index, int64_t pts, int64_t time_base);
    int Encode(AVFrame *frame, int stream_index, int64_t pts, int64_t time_base,
                         std::vector<AVPacket *> &packets);

private:
    AVCodecContext *audio_codec_ctx = NULL;
    int channels_ = 2;
    int sample_rate_ = 44100;
    int bit_rate_ = 128*1024;
    int64_t pts_ = 0;

};
#endif // AUDIOENCODER_H