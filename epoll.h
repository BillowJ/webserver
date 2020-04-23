#pragma once
#ifndef EVENTPOLL
#define EVENTPOLL
#include <string>


const int MAXEVENTS = 5000; //事件上限
const int LISTENQ = 1024;   //监听上限

int epoll_init();
int epoll_add(int epoll_fd, int fd, void *request, __uint32_t events);
int epoll_mod(int epoll_fd, int fd, void *request, __uint32_t events);
int epoll_del(int epoll_fd, int fd, void *request, __uint32_t events);
int my_epoll_wait(int epoll_fd, struct epoll_event *events, int max_evens, int timeout);

#endif
