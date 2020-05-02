#include "timer.h"
#include "epoll.h"
#include "sys/time.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>

using namespace std;


Timer::Timer(SP_ReqData _req, int _timeout) : 
    request_data(_req),
    deleted(false)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expired_time = _timeout + (now.tv_sec*1000 + now.tv_usec/1000);
}

Timer::~Timer(){
    if(request_data){
        Epoll::epoll_del(request_data->getFd());
    }
}
bool Timer::isvalid(){
    struct timeval now;
    gettimeofday(&now, NULL);
    if(expired_time > (now.tv_sec*1000 + now.tv_usec/1000)){
        return true;
    }
    else
    {
        this -> setDeleted();
        return false;
    }
}


void Timer::setDeleted()
{
    deleted = true;
    return;
}

bool Timer::isDeleted() const
{
    return deleted;
}


void Timer::clearReq()
{
    if(request_data){
        request_data.reset();
        this -> setDeleted();
    }
    return;
}
size_t Timer::getExpTime() const
{

    return expired_time;

}


TimerManager::TimerManager()
{
}

TimerManager::~TimerManager()
{
}

void TimerManager::addTimer(TimerManager::SP_ReqData request_data, int timeout)
{
    shared_ptr<Timer> timer = std::make_shared<Timer>(request_data, timeout);
    {
        MutexLockGuard locker(lock);
        TimerQueue.push(timer);
    }
    request_data -> linkTimer(timer);
}

void TimerManager::handle_expired_event()
{
    //mutex
    MutexLockGuard locker(lock);
    while(!TimerQueue.empty())
    {
        SP_Timer tmp = TimerQueue.top();
        if(tmp -> isDeleted()) //分离后被设置为删除
        {
            TimerQueue.pop();
        }
        else if(tmp -> isvalid() == false) //超时
        {
            TimerQueue.pop();
        }
        else{
            break;
        }
    }
}






