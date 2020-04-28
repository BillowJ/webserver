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
    mime['.html'] = "text/html";
    mime['.jpg'] = "image/jpeg";
    mime['.png'] = "image/png";
    mime['.gif'] = "image/gif"
    mime['.txt'] = "text/plain";
    mime['default'] = "text/html";
    mime['.ico'] = "application/x-ico";
    mime['.gz'] = "application/x-ico";
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

void RequestData::linkTimer(shared_ptr<TimerNode> _timer)
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
        shared_ptr<TimerNode> my_timer(timer.lock());
        my_timer -> clearReq();
        timer.reset(); //分离
    }
}

void RequestData::seperateTimer()
{
    if(timer.lock())
    {
        shared_ptr<TimerNode> my_timer(timer.lock());
        my_timer -> clearReq();
        timer.reset();
    }
}

void RequestData::handleRead()
{

    if(!errno)
    do
    {

    } while(false);

}

void RequestData::handleWrite()
{
    if(!error)
    {

    }

}

void RequestData::handleConn()
{

    if(!error)
    {

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
