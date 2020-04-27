#ifndef __UTIL_H_
#define __UTIL_H_
#include <cstdlib> //ssize_t
#include <string>
using namespace std;

ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer);
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &outBuffer);

void handle_for_sigpipe();
int setSocketNonBlocking(int fd);

#endif
