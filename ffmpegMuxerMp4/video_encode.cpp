#include "video.h"
extern "C"{
#include "libavutil/imgutils.h"
}

VideoEncoder::VideoEncoder(){


}

VideoEncoder::~VideoEncoder(){

    
}

int VideoEncoder::InitH264(int width, int height, int fps, int bit_rate){
    
    width_ = width;
    height_ = height;
    fps_ = fps;
    bit_rate_ = bit_rate;

    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec) {
        printf("avcodec_find_encoder AV_CODEC_ID_H264 failed\n");
        return -1;
    }
    video_codec_ctx = avcodec_alloc_context3(codec);
    if(!video_codec_ctx) {
        printf("avcodec_alloc_context3 AV_CODEC_ID_H264 failed\n");
        return -1;
    }

    video_codec_ctx->width = width_;
    video_codec_ctx->height = height_;
    video_codec_ctx->bit_rate = bit_rate_;
    //帧率（frame rate）
    video_codec_ctx->framerate = {fps_, 1};
    // 视频的 如果不主动设置：The encoder timebase is not set
    video_codec_ctx->time_base = {1, 1000000};   // 单位为微妙
    video_codec_ctx->gop_size = fps_;
    video_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    video_codec_ctx->max_b_frames = 0;
    int ret = avcodec_open2(video_codec_ctx, NULL, NULL);
    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avcodec_open2 failed:%s\n", errbuf);
        return -1;
    }
    frame_ = av_frame_alloc();
    if(!frame_) {
        printf("av_frame_alloc failed\n");
        return -1;
    }
    frame_->width = width_;
    frame_->height = height_;
    frame_->format = video_codec_ctx->pix_fmt;
    
    
    printf("Inith264 success\n");
    return 0;


}

void VideoEncoder::DeInit(){
    if(video_codec_ctx){
        avcodec_free_context(&video_codec_ctx);
    }
    if(frame_){
        av_frame_free(&frame_);
    }
}

AVCodecContext *VideoEncoder::GetCodecContext()
{
    return video_codec_ctx;
}

int VideoEncoder::Encode(uint8_t *yuv_data, int yuv_size,
                         int stream_index, int64_t pts, int64_t time_base,
                         std::vector<AVPacket *> &packets)
{
    if(!video_codec_ctx) {
        printf("codec_ctx_ null\n");
        return -1;
    }
    int ret = 0;

    pts = av_rescale_q(pts, AVRational{1, (int)time_base}, video_codec_ctx->time_base);
    frame_->pts = pts;
    if(yuv_data) {
        /*av_image_fill_arrays 本身不会验证 src 数据是否真的符合 pix_fmt 格式，
        它只会根据你传入的 pix_fmt 进行机械的计算和指针赋值。因此，即使格式不匹配，
        函数也可能返回一个 “正常” 的总字节数（只要 width、height 等参数合理），
        但这个结果对实际数据是无效的。 */
         /*av_image_fill_arrays本质是建立映射关系：将连续的图像数据缓冲区 src 按照指定的像素格式 pix_fmt 分割到不同的平面（dst_data）
        并计算每个平面的行字节数（dst_linesize）*/
        
        /*frame->data 是指向 buf 内部分区的指针数组
        frame->data 是一个指针数组（uint8_t *data[4]），
        它的作用是指向 buf 中不同像素分量的起始位置
        将连续的 buf 分割为多个 “数据平面”（plane)*/ 
        int ret_size = av_image_fill_arrays(frame_->data, frame_->linesize,
                                            yuv_data, (AVPixelFormat)frame_->format,
                                            frame_->width, frame_->height, 1);
        if(ret_size != yuv_size) {
            printf("ret_size:%d != yuv_size:%d -> failed\n", ret_size, yuv_size);
            return -1;
        }
        ret = avcodec_send_frame(video_codec_ctx, frame_);
    } else {
        ret = avcodec_send_frame(video_codec_ctx, NULL);
    }

    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avcodec_send_frame failed:%s\n", errbuf);
        return -1;
    }
    while(1)
    {
        AVPacket *packet = av_packet_alloc();
        ret = avcodec_receive_packet(video_codec_ctx, packet);
        packet->stream_index = stream_index;
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            ret = 0;
            av_packet_free(&packet);
            break;
        } else if (ret < 0) {
            char errbuf[1024] = {0};
            av_strerror(ret, errbuf, sizeof(errbuf) - 1);
            printf("h264 avcodec_receive_packet failed:%s\n", errbuf);
            av_packet_free(&packet);
            ret = -1;
        }
        printf("h264 pts:%lld\n", packet->pts);
        packets.push_back(packet);
    }
    return ret;
}

