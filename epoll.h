#pragma once
#ifndef EVENTPOLL
#define EVENTPOLL
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <sys/epoll.h>
#include "timer.h"
#include "requestData.h"
//const int MAXEVENTS = 5000; //事件上限
//const int LISTENQ = 1024;   //监听上限

// int epoll_init();
// int epoll_add(int epoll_fd, int fd, void *request, __uint32_t events);
// int epoll_mod(int epoll_fd, int fd, void *request, __uint32_t events);
// int epoll_del(int epoll_fd, int fd, void *request, __uint32_t events);
// int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_evens, int timeout);

class Epoll
{
public:
    typedef std::shared_ptr<RequestData> SP_ReqData;
private:
    static const int MAXFDS = 1000;
    static epoll_event *events;
    static int epoll_fd;
    static const std::string PATH;
    static SP_ReqData fd2req[MAXFDS];

    static TimerManager timer_manager;
public:
    static int epoll_init(int maxevents, int listen_num);
    static int epoll_add(int fd, SP_ReqData request, __uint32_t events);
    static int epoll_mod(int fd, SP_ReqData request, __uint32_t events);
    static int epoll_del(int fd, __uint32_t events = (EPOLLIN | EPOLLET | EPOLLONESHOT));
    static void my_epoll_wait(int listen_fd, int max_evens, int timeout);
    static void acceptConnection(int listen_fd, int epoll_fd, const std::string path);
    static std::vector<SP_ReqData> getEventsRequest(int listen_fd, int events_num, const std::string path);

    static void add_timer(SP_ReqData request_data, int timeout);
};


#endif
