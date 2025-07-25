#include "commonlooper.h"
#include "dlog.h"

namespace LQF
{
    //它需要一个 void* 类型的参数 p，而 this 被作为这个参数传递给线程。在线程内部，p 被转换为 CommonLooper* 类型，进而调用对象的 Loop() 方法。
void* CommonLooper::trampoline(void* p) {
    LogInfo("at CommonLooper trampoline");
    ((CommonLooper*)p)->Loop();
    return NULL;
}

CommonLooper::CommonLooper()
{
    request_exit_ = false;
}

RET_CODE CommonLooper::Start()
{
    LogInfo("at CommonLooper create");
    //this是传递给线程入口函数 trampoline 的实际参数。
    worker_ = new std::thread(trampoline, this);
    if(worker_ == NULL)
    {
        LogError("new std::thread failed");
        return RET_FAIL;
    }

    running_ = true;
    return RET_OK;
}


CommonLooper::~CommonLooper()
{
    if (running_)
    {
        LogInfo("CommonLooper deleted while still running. Some messages will not be processed");
        Stop();
    }
}


void CommonLooper::Stop()
{
    request_exit_ = true;
    if(worker_)
    {
        worker_->join();
        delete worker_;
        worker_ = NULL;
    }
    running_ = false;
}

}
