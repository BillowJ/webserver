#include <iostream>
#include <queue>
#include <deque>

#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "epoll.h"
#include "util.h"
#include "threadpool.h"

using namespace std;

int TIMER_TIME_OUT = 500;

struct epoll_event *Epoll::events;
Epoll::SP_ReqData Epoll::fd2req[MAXFDS];
int Epoll::epoll_fd = 0;
const std::string Epoll::PATH = "/"

//初始化
int Epoll::epoll_init(int maxevents, int listen_num)
{
    int epoll_fd = epoll_create(listen_num + 1);
    if(epoll_fd == -1){
        return -1;
    }
    events = new epoll_event[maxevents];
    return epoll_fd;
}

// 注册新描述符
int Epoll::epoll_add(int fd, SP_ReqData request, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;//结构体指针
    event.events = events;
    fd2req[fd] = request;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0){
        perror("epoll_add error");
        return -1;
    }
    return 0;
}

//修改已有的文件描述符
int Epoll::epoll_mod(int fd, SP_ReqData request, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    fd2req[fd] = request;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0){
        perror("epoll_mod error");
        fd2req[fd].reset();
        return -1;
    }
    return 0;
}

//从树上摘除
int Epoll::epoll_del(int fd, __uint32_t events)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = events;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0){
        perror("epoll_del error");
        return -1;
    }
    fd2req[fd].reset();
    return 0;

}

//返回活跃的事件数
void Epoll::my_epoll_wait(int listen_fd, SP_ReqData events, int max_evens, int timeout)
{
    int event_count = epoll_wait(epoll_fd, events, max_evens, timeout);
    if(event_count < 0){
        perror("epoll wait error");
    }
    std::vector<SP_ReqData> req_data = getEventsRequest(listen_fd, event_count, PATH);
    if(req_data.size() > 0)
    {
        for(auto& req : req_data)
        {
            if(ThreadPool::threadpool_add(req) < 0)
            {
                //
                break;
            }
        }
    }
    timer_manager.handle_expired_event();
}



void Epoll::acceptConnection(int listen_fd, int epoll_fd, const std::string path)
{
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    //socklen_t client_addr_len = 0;
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;
    while((accept_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len)) > 0)
    {
        cout << inet_ntoa(client_addr.sin_addr) << endl;
        cout << ntohs(client_addr.sin_port) << endl;

        // 限制服务器的最大并发连接数
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }

        // 设为非阻塞模式
        int ret = setSocketNonBlocking(accept_fd);
        if (ret < 0)
        {
            perror("Set non block failed!");
            return;
        }

        SP_ReqData req_info(new RequestData(epoll_fd, accept_fd, path));

        // 文件描述符可以读，边缘触发(Edge Triggered)模式，保证一个socket连接在任一时刻只被一个线程处理
        __uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
        Epoll::epoll_add(accept_fd, req_info, _epo_event);
        // 新增时间信息
        timer_manager.addTimer(req_info, TIMER_TIME_OUT);
    }
    //if(accept_fd == -1)
     //   perror("accept");
}

// 分发处理函数
std::vector<std::shared_ptr<RequestData>> Epoll::getEventsRequest(int listen_fd, int events_num, const std::string path)
{
    std::vector<SP_ReqData> req_data;
    for(int i = 0; i < events_num; ++i)
    {
        int fd = events[i].data.fd;

        if(fd == listen_fd)
        {
            acceptConnection(listen_fd, epoll_fd, path);
        }
        else if (fd < 3)
        {
            printf("fd < 3 \n");
            break;
        }
        else
        {
            if((events[i].events& EPOLLERR) || (events[i].events & EPOLLHUP))
            {
                printf("error event\n");
                if(fd2req[fd]) fd2req[fd] -> seperateTimer();
                fd2req[fd].reset();
                continue;
            }

            SP_ReqData cur_req = fd2req[fd];

            if(cur_req)
            {
                if((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
                    cur_req -> enableRead();
                else
                    cur_req -> enableWrite();

                cur_req -> seperateTimer();
                req_data.push_back(cur_req);
                fd2req[fd].reset();
            }
            else
            {
                cout << "SP cur_req is invalid" << endl;
            }
        }
    }
    return req_data;
    
}

void Epoll::add_timer(shared_ptr<RequestData> request_data, int timeout)
{
    timer_manager.addTimer(request_data, timeout);
}