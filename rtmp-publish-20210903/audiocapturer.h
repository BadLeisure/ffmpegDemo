#ifndef AUDIOCAPTURER_H
#define AUDIOCAPTURER_H
#include <functional>
#include "commonlooper.h"
#include "mediabase.h"
namespace LQF
{
using std::function;

class AudioCapturer : public CommonLooper
{
public:
    AudioCapturer();
    virtual ~AudioCapturer();
    /**
     * @brief Init
     * @param "audio_test": 缺省为0，为1时数据读取本地文件进行播放
     *        "input_pcm_name": 测试模式时读取的文件路径
     *        "sample_rate": 采样率
     *        "channels": 采样通道
     *        "sample_fmt": 采样格式
     * @return
     */
    RET_CODE Init(const Properties& properties);

    //virtual void Loop(); 中的 virtual 关键字表示该函数是一个虚函数，主要用于实现多态
    virtual void Loop();
    void AddCallback(function<void(uint8_t*, int32_t)> callback)
    {
        callback_get_pcm_ = callback;
    }
private:
    // 初始化参数
    int audio_test_ = 0;
    std::string input_pcm_name_;
    // PCM file只是用来测试, 写死为s16格式 2通道 采样率48Khz
    // 1帧1024采样点持续的时间21.333333333333333333333333333333ms
    int openPcmFile(const char *file_name);
    int readPcmFile(uint8_t *pcm_buf, int32_t pcm_buf_size);
    int closePcmFile();
    int64_t pcm_start_time_ = 0;    // 起始时间
    double pcm_total_duration_ = 0;        // PCM读取累计的时间
    FILE *pcm_fp_ = NULL;


    //function<void(uint8_t*, int32_t)> 这是 C++11 及以上标准中 **std::function** 模板的用法，用于封装一个 “函数对象”
    /**
     * 返回值类型：void（无返回值）
     * 参数列表：(uint8_t*, int32_t)
     * 参数列表：(uint8_t*, int32_t)
        第一个参数：uint8_t*（指向 PCM 音频数据缓冲区的指针，uint8_t 表示无符号 8 位整数，通常用于存储原始字节数据）
        第二个参数：int32_t（表示 PCM 数据的长度，单位通常是字节）
        整体表示：这个回调函数需要接收 “一段 PCM 音频数据的地址” 和 “数据长度” 作为参数，且不返回任何值
     * 
     */
    std::function<void(uint8_t*, int32_t)> callback_get_pcm_ = NULL;
    uint8_t *pcm_buf_;
    int32_t pcm_buf_size_;
    const int PCM_BUF_MAX_SIZE = 32768; //

    bool is_first_frame_ = false;

    int sample_rate_ = 48000;
};
}
#endif // AUDIOCAPTURER_H
