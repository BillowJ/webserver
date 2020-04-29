#pragma once
#include "requestData.h"
#include "./base/nocopyable.hpp"
#include "./base/mutexLock.hpp"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

class RequestData;

class Timer
{
    typedef std::shared_ptr<RequestData> SP_ReqData;
private:
    bool deleted;
    size_t expired_time;
    SP_ReqData request_data;
public:
    Timer(SP_ReqData _request_data, int timeout);
    ~Timer();
    bool isvalid();
    void clearReq();
    void setDeleted();
    bool isDeleted() const;
    size_t getExpTime() const;

};

struct timerCmp
{
    bool operator()(std::shared_ptr<Timer>& a,std::shared_ptr<Timer>& b) const
    {
        return a -> getExpTime() > b -> getExpTime();
    }
};


class TimerManager
{
    typedef std::shared_ptr<RequestData> SP_ReqData;
    typedef std::shared_ptr<Timer> SP_Timer;
private:
    std::priority_queue<SP_Timer, std::deque<SP_Timer>, timerCmp> TimerQueue;
    MutexLock lock;
public:
    TimerManager();
    ~TimerManager();
    void addTimer(SP_ReqData request_data, int timeout);
    void handle_expired_event();
};
