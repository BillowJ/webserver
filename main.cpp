#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <queue>
#include <sys/epoll.h>

#include "util.h"

using namespace std;

const int PORT = 8888;


//监听socket的初始化
int socket_bind_listen(int port){
    
    //检查port是否合法
    if(port < 1024 || port > 65535){
        return -1;
    }
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        return -1
    }
    
    int optval = 1;

    //设置地址复用,消除bind时服务器处于time_wait状态无法连接
    //"Address already in use"
    if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSERADDR, &optval, sizeof(optval)) == -1){
        return -1;
    }

    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((unsigned short)port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        return -1;
    }

    if(listen(listen_fd, LISTENQ) == -1){
        return -1;
    }
    //监听描述符非法
    if(listen_fd == -1){
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}


int main()
{
    int epoll_fd = epoll_init();
       


    return 0;
}