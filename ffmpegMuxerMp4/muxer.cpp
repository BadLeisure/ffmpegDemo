#include "muxer.h"

Muxer::Muxer()
{

}

Muxer::~Muxer()
{

}

int Muxer::Init(const char *url)
{
    //负责创建并初始化输出媒体的上下文环境，自动关联目标容器格式
    /*具体解释详见有道云笔记ffmpeg代码实战编码和解封装11 */
    int ret = avformat_alloc_output_context2(&fmt_ctx_, NULL, NULL,url);
    if(ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avformat_alloc_output_context2 failed:%s\n", errbuf);
        return -1;
    }
    url_ = url;
    return 0;
}


void Muxer::DeInit()
{
    if(fmt_ctx_) {
        avformat_close_input(&fmt_ctx_);
    }
    url_ = "";
    aud_codec_ctx_ = NULL;
    aud_stream_ = NULL;
    audio_index_ = -1;

    vid_codec_ctx_ = NULL;
    vid_stream_ = NULL;
    video_index_ = -1;
}

int Muxer::AddStream(AVCodecContext *codec_ctx)
{
    if(!fmt_ctx_) {
        printf("fmt ctx is NULL\n");
        return -1;
    }
    if(!codec_ctx) {
        printf("codec ctx is NULL\n");
        return -1;
    }
    /*
    index 是 AVStream 的重要属性，用于标识该流在 AVFormatContext 中的序号（从 0 开始）。
    当你通过 avformat_new_stream 向 fmt_ctx_（AVFormatContext）中添加新流时，FFmpeg 会根据当前已有的流数量自动分配一个唯一的 index：
    第一个创建的流 index = 0
    第二个创建的流 index = 1
    以此类推，确保每个流的 index 互不重复且连续递增
    */
   //当通过 avformat_new_stream 创建流时
   //FFmpeg 会根据输出格式（fmt_ctx_->oformat）的默认规则，为 st->time_base 分配一个初始值
    AVStream *st = avformat_new_stream(fmt_ctx_, NULL);
    if(!st) {
        printf("avformat_new_stream failed\n");
        return -1;
    }
    //    st->codecpar->codec_tag = 0;
    // 从编码器上下文复制
    //avcodec_parameters_from_context 确保 AVStream.codecpar 中包含这些必要参数
    //后续调用 avformat_write_header() 封装时，FFmpeg 会自动从 codecpar 中读取参数并写入容器
    //容器格式并不会直接从编码器中获取编码参数，它通过 AVStream 中的 codecpar 字段间接获取这些信息
    avcodec_parameters_from_context(st->codecpar, codec_ctx);
    av_dump_format(fmt_ctx_, 0, url_.c_str(), 1);

    // 判断当前的是视频流还是音频流
    if(codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
        aud_codec_ctx_ = codec_ctx;
        aud_stream_ = st;
        audio_index_ = st->index;
    }  else if(codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
        vid_codec_ctx_ = codec_ctx;
        vid_stream_ = st;
        video_index_ = st->index;
    }
    return 0;
}

int Muxer::SendHeader()
{
    if(!fmt_ctx_) {
        printf("fmt ctx is NULL\n");
        return -1;
    }
    int ret = avformat_write_header(fmt_ctx_, NULL);
    if(ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avformat_write_header failed:%s\n", errbuf);
        return -1;
    }
    return 0;
}

int Muxer::SendPacket(AVPacket *packet)
{
    int stream_index = packet->stream_index;
    printf("index:%d, pts:%lld\n", stream_index, packet->pts);

    if(!packet || packet->size <= 0 || !packet->data) {
        printf("packet is null\n");
        if(packet)
            av_packet_free(&packet);

        return -1;
    }

    AVRational src_time_base;   // 编码后的包
    AVRational dst_time_base;   // mp4输出文件对应流的time_base
    if(vid_stream_ && vid_codec_ctx_ && stream_index == video_index_) {
        src_time_base = vid_codec_ctx_->time_base;
        dst_time_base = vid_stream_->time_base;
    } else if(aud_stream_ && aud_codec_ctx_ && stream_index == audio_index_) {
        src_time_base = aud_codec_ctx_->time_base;
        dst_time_base = aud_stream_->time_base;
    }
    // 时间基转换
    packet->pts = av_rescale_q(packet->pts, src_time_base, dst_time_base);
    packet->dts = av_rescale_q(packet->dts, src_time_base, dst_time_base);
    packet->duration = av_rescale_q(packet->duration, src_time_base, dst_time_base);

    int ret = 0;
    ret = av_interleaved_write_frame(fmt_ctx_, packet); // 不是立即写入文件，内部缓存，主要是对pts进行排序
    //    ret = av_write_frame(fmt_ctx_, packet);
    av_packet_free(&packet);
    if(ret == 0) {
        return 0;
    } else {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("av_write_frame failed:%s\n", errbuf);
        return -1;
    }
}


int Muxer::SendTrailer()
{
    if(!fmt_ctx_) {
        printf("fmt ctx is NULL\n");
        return -1;
    }
    int ret = av_write_trailer(fmt_ctx_);
    if(ret != 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("av_write_trailer failed:%s\n", errbuf);
        return -1;
    }
    return 0;
}



int Muxer::Open()
{
    /**
     * 第一个参数 &fmt_ctx_->pb：指向 AVIOContext 指针的地址，函数成功后会将创建的 I/O 上下文赋值给 fmt_ctx_->pb（fmt_ctx_ 是 AVFormatContext 指针，pb 字段用于存储 I/O 上下文）
     * 第二个参数 url_.c_str()：输出目标的 URL（可以是本地文件路径，如 ./output.mp4，也可以是网络地址，如 rtmp://xxx）。
     * 第三个参数 AVIO_FLAG_WRITE：打开模式，表示以 “写入” 方式打开 I/O 上下文
     * 
     * Open() 方法的核心作用是初始化输出媒体的 I/O 通道。
     * 在 FFmpeg 封装流程中，这一步是后续写入文件头（avformat_write_header）、写入数据包（av_write_frame）、写入文件尾（av_write_trailer）的前提 
     * 只有成功打开 I/O 上下文，才能将编码后的音视频数据写入到目标文件或流中
     */
    int ret = avio_open(&fmt_ctx_->pb, url_.c_str(), AVIO_FLAG_WRITE);
    if(ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf) - 1);
        printf("avio_open %s failed:%s\n",url_.c_str(), errbuf);
        return -1;
    }
    return 0;
}

int Muxer::GetAudioStreamIndex()
{
    return audio_index_;
}


int Muxer::GetVideoStreamIndex()
{
    return video_index_;
}
