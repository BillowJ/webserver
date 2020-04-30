#include "requestData.h"
#include "util.h"
#include "epoll.h"

#include <vector>
#include <iostream>


using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string, std::string> MimeType::mime;

void MimeType::init()
{
    mime[".html"] = "text/html";
    mime[".jpg"] = "image/jpeg";
    mime[".png"] = "image/png";
    mime[".gif"] = "image/gif";
    mime[".txt"] = "text/plain";
    mime[".ico"] = "application/x-ico";
    mime[".gz"] = "application/x-ico";
    mime["default"] = "text/html";
}

std::string MimeType::getMime(const std::string& suffix)
{
    pthread_once(&once_control, MimeType::init);
    if(mime.find(suffix) == mime.end())
        return mime["defalut"];
    else
        return mime[suffix];
}

RequestData::RequestData() :
    now_read_pos(0),
    state(STATE_PARSE_URL), 
    h_state(h_start), 
    //keep_alive(false), 
    isAbleRead(true),
    isAbleWrite(false),
    events(0),
    error(false)
{
    cout << "RequestData()" << endl;
}

RequestData::RequestData(int _epollfd, int _fd, std::string _path):
    now_read_pos(0), 
    state(STATE_PARSE_URL), 
    h_state(h_start), 
    //keep_alive(false), 
    path(_path), 
    fd(_fd), 
    epollfd(_epollfd),
    isAbleRead(true),
    isAbleWrite(false),
    events(0),
    error(false)
{
    cout << "RequestData()" << endl;
}


RequestData::~RequestData()
{
    cout << "~RequestData()" << endl;
    //关闭描述符
    close(fd);
}

void RequestData::linkTimer(shared_ptr<Timer> _timer)
{
    timer = _timer;
}

int RequestData::getFd()
{
    return fd;
}

void RequestData::setFd(int _fd)
{
    fd = _fd;
}

//刷新任务对象状态
void RequestData::reset()
{
    inBuffer.clear();
    file_name.clear();
    path.clear();
    now_read_pos = 0;
    state = STATE_PARSE_URL;
    headers.clear();
    if(timer.lock())
    {
        shared_ptr<Timer> my_timer(timer.lock());
        my_timer -> clearReq();
        timer.reset(); //分离
    }
}

void RequestData::seperateTimer()
{
    if(timer.lock())
    {
        shared_ptr<Timer> my_timer(timer.lock());
        my_timer -> clearReq();
        timer.reset();
    }
}

void RequestData::handleRead()
{

    if(!errno)
    do
    {
        int read_sum = readn(fd, inBuffer);
        //read errno
        if(read_sum < 0){
            perror("readn error!");
            errno = true;
            handleError(fd, 400, "Bad Request");
            break;
        }
        else if(read_sum == 0){
            errno = true;
            break;
        }
        //success ready to next
        if(state == STATE_PARSE_URL)
        {

            
        }
        if(state == STATE_PARSE_HEADERS)
        {

        }
        if(state == STATE_RECV_BODY)
        {

        }

    } while(false);
    if(errno){
        //再次监听读
        events |= EPOLLIN;

    }
    else{
        if(outBuffer.size() > 0)
        {
            events |= EPOLLOUT;

        }
        if(state == STATE_FINISH)
        {
            if(keep_alive)
            {
                //长链接下继续监听read event
                //需要重置读缓冲区 解析状态 上一次解析的内容
                //read_pos 请求路径等
                this -> reset();
                events |= EPOLLIN;
            }

        }
        else
            events |= EPOLLIN;
        }

}

void RequestData::handleWrite()
{
    if(!error)
    {
        if(writen(fd, outBuffer) < 0)
        {
            perror("writen errno!");
            events = 0;
            error = true;
        }
        //缓冲区还有数据
        else if(outBuffer.size() > 0)
            events |= EPOLLOUT;
    }

}

void RequestData::handleConn()
{

    if (!error)
    {
        int timeout = 1000;
        if(keep_alive) timeout *= 500;
        //io完毕 初始化 通过分发函数进行重新判定
        isAbleRead = false;
        isAbleWrite = false;
        //加入请求队列前加入定时器
        Epoll::add_timer(shared_from_this(), timeout);
        //判断当前事件状态是否需要写回
        if((events & EPOLLIN) && (events & EPOLLOUT))
        {
            events = __uint32_t(0);
            events |= EPOLLOUT;
        }
        else
        {
            events = __uint32_t(0);
            events |= (EPOLLIN | EPOLLONESHOT);
        }
        events |= EPOLLET;
        __uint32_t _events = events;
        events = __uint32_t(0);
        int res = Epoll::epoll_mod(fd, shared_from_this(), _events);
        if(res < 0 )
        {
            printf("epoll_mod failed!");
        }
    }
    else if (keep_alive)
    {
        isAbleRead = false;
        isAbleWrite = false;
        events = __uint32_t(0);
        events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
        int _events = events;
        events = 0;
        int timeout = 500 * 1000;
        Epoll::add_timer(shared_from_this(), timeout); 
        int res = Epoll::epoll_mod(fd, shared_from_this(), _events);
        if(res < 0 )
        {
            printf("epoll_mod failed!");
        }
    }
}

int RequestData::parse_URL()
{

    string &str = inBuffer;
}

int RequestData::parse_Headers()
{
    string &str = inBuffer;

}

int RequestData::analysisRequest()
{
    if(method == METHOD_GET)
    {

        return ANALYSIS_SUCCESS;
    }
    else
    {
        return ANALYSIS_ERROR;
    }
}

void RequestData::handleError(int fd, int err_num, string short_msg)
{

}

void RequestData::disableReadAndWrite()
{
    isAbleRead = false;
    isAbleWrite = false;
}

void RequestData::enableRead()
{
    isAbleRead = true;
}
void RequestData::enableWrite()
{
    isAbleWrite = true;
}
bool RequestData::canRead()
{
    return isAbleRead;
}
bool RequestData::canWrite()
{
    return isAbleWrite;
}
