#include "requestData.h"
#include "util.h"
#include "epoll.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cstdlib>
#include <unistd.h>
#include <queue>
#include <iostream>
#include <cstring>
#include <string>
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
    keep_alive(false), 
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
    keep_alive(false), 
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

    cout << "handle read" << endl;
    do
    {
        int read_sum = readn(fd, inBuffer);
        //read errno
        cout << "read_sum:" << read_sum << endl;
        if(read_sum < 0){
            perror("readn error!");
            error = true;
            handleError(fd, 400, "Bad Request");
            break;
        }
        else if(read_sum == 0){
            cout <<"read_sum == 0 error" << endl;
            error = true;
            break;
        }
        //success ready to next
        if(state == STATE_PARSE_URL)
        {
            int flag = this -> parse_URL();
            if(flag == PARSE_URI_AGAIN)
                break;
            else if(flag == PARSE_URI_ERROR)
            {
                perror("parse url error!");
                error = true;
                handleError(fd, 400, "Bad Request");
                break;
            }
            else
                //parse url finish
                cout << "Parse URL Finish" << endl;
            state = STATE_PARSE_HEADERS;

        }
        if(state == STATE_PARSE_HEADERS)
        {
            int flag = this -> parse_Headers();
            if(flag == PARSE_HEADER_AGAIN)
            {
                cout << "parse_header again." << endl;
                break;
            }
            else if(flag == PARSE_HEADER_ERROR)
            {
                perror("3");
                error = true;
                handleError(fd, 400, "Bad Request");
                break;
            }
            if(method == METHOD_GET)
                state = STATE_ANALYSIS;
            else 
                state = STATE_RECV_BODY;

        }
        if(state == STATE_RECV_BODY)
        {
            
        }
        if(state == STATE_ANALYSIS)
        {
            cout <<"analysisRequest" << endl;
            int flag = this -> analysisRequest();
            if(flag == ANALYSIS_SUCCESS)
            {
                cout << "state_finish" << endl;
                state = STATE_FINISH;
                break;
            }
            else
            {
                cout << "analysis error!" << endl;
                error = true;
                break;
            }
        }

    } while(false);
    cout << "error: " << error << endl;
   /*
    if(error){
        //再次监听读
        events |= EPOLLIN;

    }
    */
    if(!error)
    {
        cout << "ready to write" << endl;
        if(outBuffer.size() > 0)
        {
            cout << "EPOLLOUT"<< endl;
            events |= EPOLLOUT;
        }
        //一开始判定了错误的返回状态 导致出现死循环写.
        if(state == STATE_FINISH)
        {
            cout << "keep-alive=" << keep_alive << endl;
            if(keep_alive)
            {
                //长链接下继续监听read event
                //需要重置读缓冲区 解析状态 上一次解析的内容
                //read_pos 请求路径等
                this -> reset();
                events |= EPOLLIN;
            }
            else
                return;
        }
        else
            events |= EPOLLIN;
    }

}

void RequestData::handleWrite()
{
    cout << "handle write" << endl;
    if(!error)
    {
        if(writen(fd, outBuffer) < 0)
        {
            perror("writen falied!");
            events = 0;
            error = true;
        }
        //缓冲区还有数据
        else if(outBuffer.size() > 0){
            cout << "writing..." << endl;
            events |= EPOLLOUT;
        }
        /*
         不可以如果写完直接置成EPOLLIN 如果是短连接直接断开置为空
        else{
            events |= EPOLLIN;
        }
        */
    }

}

void RequestData::handleConn()
{
    cout << "handleConn" <<endl;
    if (!error)
    {//这里遇到一个大坑 没判断当前的event是否为空

        if(events != 0)
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
                cout <<"bug test" << endl;
                events = __uint32_t(0);
                events |= EPOLLOUT;
            }
            /*
            else
            {
                events |= EPOLLIN;
            }
            */
            events |= (EPOLLET | EPOLLONESHOT);
            
            __uint32_t _events = events;
            events = 0;
            int res = Epoll::epoll_mod(fd, shared_from_this(), _events);
            if(res < 0 )
            {
                printf("epoll_mod failed!");
            }
        }
        else if (keep_alive)
        {
            cout << " keep_alive" << endl;
            isAbleRead = false;
            isAbleWrite = false;
            events = __uint32_t(0);
            events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
            __uint32_t  _events = events;
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
}

int RequestData::parse_URL()
{

    string &str = inBuffer;
    int pos = str.find('\r', now_read_pos);
    //没解析到内容
    if(pos < 0){
        state = PARSE_URI_AGAIN;
    }
    //"GET /index HTTP/1.1
    string _line = str.substr(0, pos);
    if(str.size() > pos+1)
       str = str.substr(pos+1);
    else
        str.clear(); //finish
    pos = _line.find("GET");
    if(pos == -1)
    {
        pos = _line.find("POST");
        if(pos == -1)
            return PARSE_URI_ERROR;
        else
            method = METHOD_POST;
    }
    else
    {
        cout << "Method:GET" << endl;
        method = METHOD_GET;
    }
    pos = _line.find("/", pos);
    if(pos == -1)
        return PARSE_URI_ERROR;
    else
    {   
        //filename
        int _pos = _line.find(' ', pos);
        if(_pos == -1)
            return PARSE_URI_ERROR;
        else
        {
            if(_pos - pos > 1){
                file_name = _line.substr(pos + 1, _pos - (pos + 1));
                int __pos = file_name.find('?');
                if(__pos >= 0)
                    file_name = file_name.substr(0, __pos);
                    std::cout << file_name << std::endl;
            }
            else file_name = "index.html";
        }
        pos = _pos; // ' '
    }
    
    //"GET /index HTTP/1.1 
    //prase http version
    pos = _line.find('/', pos);
    if(pos == -1)
    {
        return PARSE_URI_ERROR;
    }
    else
    {
        string ver = _line.substr(pos+1, 3);
        if(ver == "1.0")
            HTTPversion = HTTP_10;
        else if(ver == "1.1")
            HTTPversion = HTTP_11;
        else
            return PARSE_URI_ERROR;
    }
    
    return PARSE_URI_SUCCESS;
    
}



int RequestData::parse_Headers()
{
    string &str = inBuffer;
    cout << str << endl;
    int key_start = -1;
    int key_end = -1;
    int val_start = -1;
    int val_end = -1;
    int now_pos = 0;
    bool notFinish = true;
    for(int i = 0; i < str.size() && notFinish; ++i)
    {
        switch(h_state)
        {
            case h_start:
            {
                cout << str[0] << endl;
                if(str[i] == '\n' || str[i] == '\r')
                    break;
                h_state = h_key;
                key_start = i;
                now_pos = i;
                break;
            }
            case h_key:
            {
                if(str[i] == ':')
                {
                    key_end = i;
                    if(key_end - key_start <= 0)
                        return PARSE_HEADER_ERROR;
                    h_state = h_colon;
                }
                /*
                else if (str[i] == '\n' || str[i] == '\r');
                {
                    cout << "i: " << i << " " << endl;
                    cout << "str[i] " << str[i] << endl;
                    cout << "test" << endl;
                    return PARSE_HEADER_AGAIN;
                }*/
                break;
            }
            case h_colon:
            {
                if(str[i] == ' ')
                {
                    h_state = h_spaces_after_colon;

                }
                else
                {
                    return PARSE_HEADER_ERROR;
                }
                break;
            }
            case h_spaces_after_colon:
            {
                h_state = h_value;
                val_start = i;
                break;
            }
            case h_value:
            {
                if(str[i] == '\r')
                {
                    val_end = i;
                    h_state = h_CR;
                    if(val_end - val_start <= 0)
                    {
                        return PARSE_HEADER_ERROR;
                    }
                }
                break;
            }
            case h_CR:
            {
                if(str[i] = '\n')
                {
                    h_state = h_LF;
                    string key(str.begin() + key_start, str.begin() + key_end);
                    string value(str.begin() + val_start, str.begin() + val_end);
                    headers[key] = value;
                    cout << "key: " << key << endl;
                    cout << "val: " << value << endl;
                    now_pos = i;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_LF:
            {
                if(str[i] == '\r')
                {
                    h_state = h_end_CR;
                }
                else
                {
                    key_start = i;
                    h_state = h_key;
                }
                break;
            }
            case h_end_CR:
            {
                if(str[i] == '\n')
                {
                    h_state = h_end_LF;
                }
                else
                    return PARSE_HEADER_ERROR;
                break;
            }
            case h_end_LF:
            {
                notFinish = false;//finish
                key_start = i;
                now_pos = i;
                break;
            }
        }
    }
    if(h_state == h_end_LF)
    {
        str = str.substr(now_pos);
        return PARSE_HEADER_SUCCESS;
    }
    else
    {
        str = str.substr(now_pos);
        return PARSE_HEADER_AGAIN;
    }
}

int RequestData::analysisRequest()
{
    if(method == METHOD_GET)
    {
        string header;
        header += "HTTP/1.1 200 OK\r\n";
        if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive")
        {
            keep_alive = true;
           // header += string("Connection: keep-alive\r\n") + "Keep-alive: timeout=" + ‘"500 * 1000" + "\r\n";
        
            header += string("Connection: keep-alive\r\n") + "Keep-alive: timeout=";
            header += to_string(500 * 1000);
            header += "\r\n";
        }
        int dot_pos = file_name.find('.');
        string filetype;
        if(dot_pos >= 0)
            filetype = MimeType::getMime(file_name.substr(dot_pos));
        else
            filetype = MimeType::getMime("default");
        struct stat sbuf;
        if(stat(file_name.c_str(), &sbuf) < 0)
        {
            header.clear();
            cout << "file not exist" << endl;
            handleError(fd, 404, "Not Found!");
            return ANALYSIS_ERROR;
        }
        header += "Content-type: " + filetype + "\r\n";
        header += "Content-length: " + to_string(sbuf.st_size) + "\r\n";
        header += "\r\n";
        outBuffer += header;
        int src_fd = open(file_name.c_str(), O_RDONLY, 0);
        char *src_addr = (char *)mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
        close(src_fd);
        outBuffer += src_addr;
        munmap(src_addr, sbuf.st_size);
        cout << " file anay... succerss!" << endl;
        return ANALYSIS_SUCCESS;
    }
    else
    {
        return ANALYSIS_ERROR;
    }
}

void RequestData::handleError(int fd, int err_num, string short_msg)
{
    short_msg = " " + short_msg;
    char send_buf[MAX_BUFF];
    string body_buf;
    string header_buf;
    
    body_buf += "<html><title>Erro!!!!!!!!!!</title>";
    body_buf += "<body bgcolor=\"ffffff\">";
    body_buf += to_string(err_num) + short_msg;
    body_buf += "<hr><em> Web Server...</em>\n</body></html>";
    
    header_buf += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    header_buf += "Content-type: text/html\r\n";
    header_buf += "Connection: close\r\n";
    header_buf += "Content-length: " + to_string(body_buf.size()) + "\r\n";
    header_buf += "\r\n";
    
    //cout << body_buf << endl;
    sprintf(send_buf, "%s", header_buf.c_str());
    //cout << "strlen(send_buf)"<<strlen(send_buf) << endl;
    //cout << "cur_fd = " << fd << endl;
    writen(fd, send_buf, strlen(send_buf));
    
    sprintf(send_buf, "%s", body_buf.c_str());
    writen(fd, send_buf, strlen(send_buf));

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
