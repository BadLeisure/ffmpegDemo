#include <iostream>
#include "audio.h"


AudioEncode::AudioEncode(){

}

AudioEncode::~AudioEncode(){
    DeInit();
}


int AudioEncode::initAAC(int channels,int sampleRate,int bit_rate){

    channels_ = channels;
    sample_rate_ = sampleRate;
    bit_rate_ = bit_rate;
    //这里现在默认用aac编码
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);

    //创建与该编码器关联的上下文,codec_ctx->codec_type自动赋值：通过 avcodec_alloc_context3() 关联编码器时
    audio_codec_ctx = avcodec_alloc_context3(codec);
    if(!audio_codec_ctx) {
        printf("avcodec_alloc_context3 AV_CODEC_ID_AAC failed\n");
        return -1;
    }
    
    //不加adts帧头部
    audio_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    audio_codec_ctx->bit_rate = bit_rate_;
    audio_codec_ctx->channels = channels_;
    //表示音频的采样率
    audio_codec_ctx->sample_rate = sample_rate_;
    audio_codec_ctx->channel_layout = av_get_default_channel_layout(channels_);
    audio_codec_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    audio_codec_ctx->time_base = AVRational{1,sample_rate_};
    
    /*若 codec_ctx_ 未提前关联编码器（即通过 avcodec_alloc_context3(NULL) 创建）
    此时传递第二个参数是NULL 会导致 avcodec_open2 失败*/
    /* 当传递 NULL 时，表示：不传递额外配置参数，仅使用 codec_ctx_ 中已设置的参数（如宽高、帧率、码率等）初始化编码器*/
   
    //函数原型：int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options);
    int ret = avcodec_open2(audio_codec_ctx, NULL, NULL);
     if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avcodec_open2 failed:%s\n", errbuf);
        return -1;
    }
    printf("InitAAC success\n");
    return 0;
}
/**
 * int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq);
    功能：将数值 a 从 bq 时间基准转换到 cq 时间基准，计算公式为：
    result = a × bq ÷ cq 
 */

AVPacket *AudioEncode::encode(AVFrame *frame, int stream_index, int64_t pts, int64_t time_base)
{
    if(!audio_codec_ctx) {
        printf("codec_ctx_ null\n");
        return NULL;
    }
    //将时间戳 pts 从原始时间基准（AVRational{1, (int)time_base}）转换为编码器上下文（codec_ctx_）使用的时间基准
    pts = av_rescale_q(pts, AVRational{1, (int)time_base}, audio_codec_ctx->time_base);
    if(frame) {
        frame->pts = pts;
    }
    int ret = avcodec_send_frame(audio_codec_ctx, frame);
    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avcodec_send_frame failed:%s\n", errbuf);
        return NULL;
    }
    AVPacket *packet = av_packet_alloc();
    ret = avcodec_receive_packet(audio_codec_ctx, packet);
    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("aac avcodec_receive_packet failed:%s\n", errbuf);
        av_packet_free(&packet);
        return NULL;
    }
    packet->stream_index = stream_index;
    return packet;
}

int AudioEncode::Encode(AVFrame *frame, int stream_index, int64_t pts, int64_t time_base,
                         std::vector<AVPacket *> &packets)
{
    if(!audio_codec_ctx) {
        printf("codec_ctx_ null\n");
        return NULL;
    }
    pts = av_rescale_q(pts, AVRational{1, (int)time_base}, audio_codec_ctx->time_base);
    if(frame) {
        frame->pts = pts;
    }
    int ret = avcodec_send_frame(audio_codec_ctx, frame);
    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avcodec_send_frame failed:%s\n", errbuf);
        return NULL;
    }
    while(1)
    {
        AVPacket *packet = av_packet_alloc();
        ret = avcodec_receive_packet(audio_codec_ctx, packet);
        //会把AVPacket和AVStream关联上
        packet->stream_index = stream_index;
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            av_packet_free(&packet);
            break;
        } else if (ret < 0) {
            char errbuf[1024] = {0};
            av_strerror(ret, errbuf, sizeof(errbuf) - 1);
            printf("aac avcodec_receive_packet failed:%s\n", errbuf);
            av_packet_free(&packet);
            ret = -1;
        }
        packets.push_back(packet);
    }
    return ret;
}

AVCodecContext* AudioEncode::GetCodecContext(){
    return audio_codec_ctx;
}

int AudioEncode::GetChannels(){

    if(audio_codec_ctx){}
        return audio_codec_ctx->channels;
     return -1;
}


int AudioEncode::GetSampleFormat(){
    if(audio_codec_ctx){}
        return audio_codec_ctx->sample_fmt;
     return -1;
}

//一帧音频中每个声道所包含的样本数量,通过avcodec_open2()打开编码器后会被自动设置
int AudioEncode::GetFrameSize(){
    if(audio_codec_ctx){}
        return audio_codec_ctx->frame_size;
     return -1;
}

void AudioEncode::DeInit(){
    if(audio_codec_ctx){
        avcodec_free_context(&audio_codec_ctx);
    }
}
