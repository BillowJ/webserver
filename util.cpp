#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <iostream>
using namespace std;

const int MAX_BUFF = 4096;

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
ssize_t readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while(true)
    {
        char buff[MAX_BUFF];
        if((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if(errno == EINTR){
                continue;
            }
            else if(errno == EAGAIN)
            {
                return readSum;
            }
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if(nread == 0)
            break;
        readSum += nread;
        inBuffer += std::string(buff, buff+nread);
    }
    return readSum;
}


ssize_t writen(int fd, void *buff, size_t n)
{
    size_t writeSum = 0;
    size_t nwritted = 0;
    size_t nleft = n;
    char *ptr = (char *)buff;
    while(nleft > 0)
    {
        if((nwritted = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritted < 0)
            {
                if(errno == EINTR)
                {
                    nwritted = 0;
                    continue;
                }
                else if(errno == EAGAIN)
                {
                    return writeSum;
                }
                else
                    return -1;
            }
        }
        writeSum += nwritted;
        nleft -= nwritted;
        ptr += nwritted;
    }
    //cout << "writeSum" << writeSum << endl;
    /*
    if(writeSum == buff.size())
        buff.clear();
    else
        buff = buff.substr(writeSum);
    */
    return writeSum;
}

ssize_t writen(int fd, std::string &buff)
{
    ssize_t nleft = buff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = buff.c_str();
    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if(nwritten < 0)
            {
                if(errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if(errno == EAGAIN)
                {
                    break;
                }
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if(writeSum == buff.size())
        buff.clear();
    else
        buff = buff.substr(writeSum);
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
