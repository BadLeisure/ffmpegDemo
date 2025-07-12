#include <iostream>
using namespace std;
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 音频解码，将aac流解码为pcm
 * 
 */



#define AUDIO_INBUF_SIZE 20480
//4096就是一帧数据的大小
#define AUDIO_REFILL_THRESH 4096

static char err_buf[128] = {0};




bool isMP3(const string &filename) {
    // 转换为小写以便不区分大小写比较
    std::string ext = filename;
    std::transform(ext.begin(), ext.end(), ext.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    // 查找扩展名
    size_t dotPos = ext.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = ext.substr(dotPos + 1);
        return (extension == "mp3");
    }
    return false;
}

bool isAAC(const string &filename) {
    // 同理，处理AAC扩展名（常见为.aac或.m4a）
    std::string ext = filename;
    std::transform(ext.begin(), ext.end(), ext.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    size_t dotPos = ext.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = ext.substr(dotPos + 1);
        return (extension == "aac" || extension == "m4a");
    }
    return false;
}

static void print_sample_format(const AVFrame *frame)
{
    printf("ar-samplerate: %uHz\n", frame->sample_rate);
    printf("ac-channel: %u\n", frame->channels);
    printf("f-format: %u\n", frame->format);// 格式需要注意，实际存储到本地文件时已经改成交错模式
}

static char* av_get_err(int errnum)
{
    av_strerror(errnum, err_buf, 128);
    return err_buf;
}

static void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame,
                   FILE *outfile)
{
    int i, ch;
    int ret, data_size;
    int frame_from_avpacket_size;
    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if(ret == AVERROR(EAGAIN))
    {
        fprintf(stderr, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
    }
    else if (ret < 0)
    {
        fprintf(stderr, "Error submitting the packet to the decoder, err:%s, pkt_size:%d\n",
                av_get_err(ret), pkt->size);
//        exit(1);
        return;
    }

    /* read all the output frames (infile general there may be any number of them */
    while (ret >= 0)
    {
        // 对于frame, avcodec_receive_frame内部每次都先调用
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0)
        {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        frame_from_avpacket_size++;
        cout << "frame_from_avpacket_size:" << frame_from_avpacket_size << "ret:" << ret << endl;
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0)
        {
            /* This should not occur, checking just for paranoia */
            fprintf(stderr, "Failed to calculate data size\n");
            exit(1);
        }
        static int s_print_format = 0;
        if(s_print_format == 0)
        {
            s_print_format = 1;
            //就打印一次，为了测试用
            print_sample_format(frame);
        }
        /**
            P表示Planar（平面），其数据格式排列方式为 :data[0]存放L  data[1]存放R
            LLLLLLRRRRRRLLLLLLRRRRRRLLLLLLRRRRRRL...（每个LLLLLLRRRRRR为一个音频帧）
            而不带P的数据格式（即交错排列）排列方式为：
            LRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRLRL...（每个LR为一个音频样本）
         播放范例：   ffplay -ar 48000 -ac 2 -f f32le believe.pcm
          */
         //frame->nb_samples一帧的样本总数
        for (i = 0; i < frame->nb_samples; i++)
        {
            for (ch = 0; ch < dec_ctx->channels; ch++)  // 交错的方式写入, 大部分float的格式输出
              //data_size:每个样本的字节数
            fwrite(frame->data[ch] + data_size*i, 1, data_size, outfile);
        }
    }
}
 

int main(){
    
    const char *fileInName = "D:\\mediaExercise\\test.aac";
    const char *filenOutName;
    AVCodecContext *codec_ctx = NULL;
    AVCodec *codec = NULL;
    AVCodecParserContext *parser = NULL;
    AVPacket *pkt = NULL;
    AVFrame *decoded_frame = NULL;
    int len = 0;
    int ret = 0;

    //定义一个输入缓冲区
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    //指向该处理的位置
    uint8_t *data = NULL;
    size_t  data_size = 0;

    FILE *infile = NULL;
    FILE *outfile = NULL;

    enum AVCodecID audio_codec_id = AV_CODEC_ID_AAC;

    //以二进制形式打开一个文件，并且仅用于读取操作
    infile = fopen(fileInName,"rb");
    if(!infile){
        cout << "open infile fail" << endl; 
        exit(1);
    }
    //以二进制形式打开一个文件，并且仅用于写入操作
    outfile = fopen("D:\\mediaExercise\\out.pcm","wb");

    if(!outfile){
        cout << "open outfile fail" << endl; 
        exit(1);
    }

    if(isAAC(fileInName)){
        audio_codec_id = AV_CODEC_ID_AAC;

    }else if(isMP3(fileInName)){
        audio_codec_id = AV_CODEC_ID_MP3;

    }
    codec = avcodec_find_decoder(audio_codec_id);
    if(!codec){
        cout << "Codec not found" << endl; 
        exit(1);
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx){
        cout << "avcodec_alloc_context3 fail" << endl;
        exit(1);
    }

     ret = avcodec_open2(codec_ctx,codec,NULL);
    if(ret < 0){
        cout << "avcodec_open2 fail";
       exit(1);
    }

    //解析器的核心价值在于自动提取和解码器所需的动态参数，并处理流中的各种复杂情况。虽然在严格控制的条件
    //下可以手动指定参数，但这会牺牲代码的健壮性和通用性。对于大多数应用场景，使用解析器是更安全的选择
    // 获取裸流的解析器 AVCodecParserContext(数据)  +  AVCodecParser(方法)
    parser = av_parser_init(codec->id);

    data      = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);

    while (data_size > 0)
    {
        if (!decoded_frame)
        {
            if (!(decoded_frame = av_frame_alloc()))
            {
                fprintf(stderr, "Could not allocate audio frame\n");
                exit(1);
            }
        }

       
        //data参数的核心作用是标记原始输入数据的处理进度，而解析器的内部缓存负责存储已解析但未输出的帧
       //ret表示表示从data中读取并处理的总字节数，不一定就是一帧长度，可能小于一帧，也可能大于一帧
        ret = av_parser_parse2(parser, codec_ctx, &pkt->data, &pkt->size,
                               data, data_size,
                               AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0)
        {
            fprintf(stderr, "Error while parsing\n");
            exit(1);
        }
        data      += ret;   // 跳过已经解析的数据
        data_size -= ret;   // 对应的缓存大小也做相应减小

        if (pkt->size)
            decode(codec_ctx, pkt, decoded_frame, outfile);

        if (data_size < AUDIO_REFILL_THRESH)    // 如果数据少了则再次读取
        {
            memmove(inbuf, data, data_size);    // 把之前剩的数据拷贝到buffer的起始位置
            data = inbuf;
            // 读取数据 长度: AUDIO_INBUF_SIZE - data_size
            len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
            if (len > 0)
                data_size += len;
        }
    }


 /* 冲刷解码器 */
    pkt->data = NULL;   // 让其进入drain mode
    pkt->size = 0;
    decode(codec_ctx, pkt, decoded_frame, outfile);

    fclose(outfile);
    fclose(infile);

    avcodec_free_context(&codec_ctx);
    av_parser_close(parser);
    av_frame_free(&decoded_frame);
    av_packet_free(&pkt);

    printf("main finish, please enter Enter and exit\n");
    return 0;



}


