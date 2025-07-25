#include "Looper.h"
#include "dlog.h"
#include "timeutil.h"
#include "avtimebase.h"
namespace LQF
{
void* Looper::trampoline(void* p)
{
    LogInfo("at Looper trampoline");
    ((Looper*)p)->loop();
    return NULL;
}

Looper::Looper(const int deque_max_size)
    :deque_max_size_(deque_max_size)
{
    LogInfo("at Looper create");
    head_data_available_ = new Semaphore(0);
    worker_ = new std::thread(trampoline, this);
    running_ = true;
}


Looper::~Looper()
{
    if (running_)
    {
        LogInfo("Looper deleted while still running. Some messages will not be processed");
        Stop();
    }
}

//
void Looper::Post(int what, void *data, bool flush)
{
    LooperMessage *msg = new LooperMessage();
    msg->what = what;
    msg->obj = (MsgBaseObj *)data;
    msg->quit = false;
    addmsg(msg, flush);
}

void Looper::addmsg(LooperMessage *msg, bool flush)
{
    // 获取锁，发数据
    int64_t t1 = TimesUtil::GetTimeMillisecond();
    queue_mutex_.lock();
    //防止队列积压过多消息导致内存溢出。
    // flush=true：主动要求清空队列（例如，切换播放内容时，需丢弃旧消息）
    if (flush || msg_queue_.size() > deque_max_size_) {
        LogWarn("flush queue,what:%d, size:%d, t:%u",
                msg->what, msg_queue_.size(), AVPlayTime::GetInstance()->getCurrenTime());
        while(msg_queue_.size() > 0)
        {
            LooperMessage * tmp_msg = msg_queue_.front();
            msg_queue_.pop_front();
            delete tmp_msg->obj;
            delete tmp_msg;
        }
    }

    msg_queue_.push_back(msg);
    queue_mutex_.unlock();
    // 发送数据进行通知
    if(1 == msg->what)
    LogInfo("post msg %d, size:%d, t:%u", msg->what, msg_queue_.size(), AVPlayTime::GetInstance()->getCurrenTime());

    //head_data_available_->post()：这是线程同步的关键，通常 head_data_available_ 是一个信号量,用于通知等待中的 “消费者线程”（如消息处理线程）：队列中有新消息，可立即处理。
    /**
     * 信号量的计数器 加 1（表示 “可用资源 / 事件数量增加”）。
       如果有线程正在通过 head_data_available_->wait() 等待 “消息队列有数据”，则该线程会被唤醒，从而可以从队列中读取并处理新添加的消息
     */
    head_data_available_->post();
    //    LogInfo("Looper post");
    int64_t t2 = TimesUtil::GetTimeMillisecond();
    if(t2 - t1 >10)
    {
        LogWarn("t2 - t1 = %ld", t2-t1);
    }
}

void Looper::loop()
{
    LogInfo("into loop");
    LooperMessage *msg;
    while(true)
    {
        queue_mutex_.lock();
        int size = msg_queue_.size();
        if(size > 0)
        {
            msg = msg_queue_.front();
            msg_queue_.pop_front();
            queue_mutex_.unlock();
            //quit 退出
            if (msg->quit)
            {
                break;
            }
            if(msg->what == 1)
            LogDebug("processing into msg %d, size:%d t:%u", msg->what, size,
                    AVPlayTime::GetInstance()->getCurrenTime());
            handle(msg->what, msg->obj);
            if(msg->what == 1)
            LogDebug("processing leave msg %d, size:%d t:%u", msg->what, size,
                    AVPlayTime::GetInstance()->getCurrenTime());
            delete msg;
//            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        else {
            queue_mutex_.unlock();
//            if(msg->what == 1)
            LogDebug("sleep msg, t:%u", AVPlayTime::GetInstance()->getCurrenTime());
            head_data_available_->wait();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
//        else
//        {
//            queue_mutex_.unlock();
//            //            LogInfo("no msg");
//            head_data_available_->wait();
//            LogDebug("wait get msg:%d", msg_queue_.size());
//            continue;
//        }

    }
    delete msg->obj;
    delete msg;
    while(msg_queue_.size() > 0)
    {
        msg = msg_queue_.front();
        msg_queue_.pop_front();
        delete msg->obj;
        delete msg;
    }
}

void Looper::Stop()
{
    if(running_)
    {
        LogInfo("Stop");
        LooperMessage *msg = new LooperMessage();
        msg->what = 0;
        msg->obj = NULL;
        msg->quit = true;
        addmsg(msg, true);
        if(worker_)
        {
            worker_->join();
            delete worker_;
            worker_ = NULL;
        }
        if(head_data_available_)
            delete head_data_available_;

        running_ = false;
    }
}

void Looper::handle(int what, void* obj)
{
    LogInfo("dropping msg %d %p", what, obj);
}
}
