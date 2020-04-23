#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


ssize_t readn(int fd, void *buff, size_t n){
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while(nleft > 0){
        if((nread = read(fd, ptr, nleft)) < 0)
        {
            if(errno == EINTR) 
                nread = 0;
            else if (errno == EAGAIN){
                return readSum;
            }
            else{
                return -1;
            }
        }else if(nread == 0) break;
        readSum += nread;
        nleft -= nread;
        ptr += nread;   //地址后移
    }
    return readSum;
}


ssize_t writen(int fd, void *buff, size_t n){
    int writeSum = 0;
    int nwritted = 0;
    int nleft = n;
    char *ptr = (char *)buff;
    while(nleft > 0){
        if((nwritted = read(fd, ptr, nleft)) <= 0){
            if(nwritted < 0){
                if(errno == EINTR || errno == EAGAIN){
                    nwritted = 0;
                    continue;
                }else{
                    return -1;
                }
            }
        }
        writeSum += nwritted;
        nleft -= nwritted;
        ptr += nwritted;
    }
    return writeSum;
}

// https://blog.csdn.net/u010821666/article/details/81841755
// 关于SIGPIPE的处理及其产生原因
//SIG_IGN https://blog.csdn.net/u012317833/article/details/39253793 

void handle_for_sigpipe(){
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if(sigaction(SIGPIPE, &sa, NULL)) 
        return;
}

int setSocketNonBlocking(int fd){
    int flag = fcntl(fd, F_GETFL, 0);
    if(flag == -1) return -1;
    flag |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flag) == -1)
        return -1;
    return 0;
}
