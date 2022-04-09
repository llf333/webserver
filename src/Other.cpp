// Created by llf on 2022/3/11.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "Other.h"

///////////////
//Global Value//
///////////////




int GlobalValue::CurrentUserNumber=0;
std::mutex GlobalValue::usernumber_mtx{};
std::chrono::seconds GlobalValue::HttpHEADTime=std::chrono::seconds(20);
std::chrono::seconds GlobalValue::HttpPostBodyTime=std::chrono::seconds(60);
std::chrono::seconds GlobalValue::keep_alive_time=std::chrono::seconds(60);
int GlobalValue::BufferMaxSize=2048;
int GlobalValue::TimeWheel_PerSlotTime=1;

char GlobalValue::Favicon[555] = {//复制别人的echo test，至于是什么还不清楚
        '\x89', 'P',    'N',    'G',    '\xD',  '\xA',  '\x1A', '\xA',  '\x0',
        '\x0',  '\x0',  '\xD',  'I',    'H',    'D',    'R',    '\x0',  '\x0',
        '\x0',  '\x10', '\x0',  '\x0',  '\x0',  '\x10', '\x8',  '\x6',  '\x0',
        '\x0',  '\x0',  '\x1F', '\xF3', '\xFF', 'a',    '\x0',  '\x0',  '\x0',
        '\x19', 't',    'E',    'X',    't',    'S',    'o',    'f',    't',
        'w',    'a',    'r',    'e',    '\x0',  'A',    'd',    'o',    'b',
        'e',    '\x20', 'I',    'm',    'a',    'g',    'e',    'R',    'e',
        'a',    'd',    'y',    'q',    '\xC9', 'e',    '\x3C', '\x0',  '\x0',
        '\x1',  '\xCD', 'I',    'D',    'A',    'T',    'x',    '\xDA', '\x94',
        '\x93', '9',    'H',    '\x3',  'A',    '\x14', '\x86', '\xFF', '\x5D',
        'b',    '\xA7', '\x4',  'R',    '\xC4', 'm',    '\x22', '\x1E', '\xA0',
        'F',    '\x24', '\x8',  '\x16', '\x16', 'v',    '\xA',  '6',    '\xBA',
        'J',    '\x9A', '\x80', '\x8',  'A',    '\xB4', 'q',    '\x85', 'X',
        '\x89', 'G',    '\xB0', 'I',    '\xA9', 'Q',    '\x24', '\xCD', '\xA6',
        '\x8',  '\xA4', 'H',    'c',    '\x91', 'B',    '\xB',  '\xAF', 'V',
        '\xC1', 'F',    '\xB4', '\x15', '\xCF', '\x22', 'X',    '\x98', '\xB',
        'T',    'H',    '\x8A', 'd',    '\x93', '\x8D', '\xFB', 'F',    'g',
        '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f',    'v',    'f',    '\xDF',
        '\x7C', '\xEF', '\xE7', 'g',    'F',    '\xA8', '\xD5', 'j',    'H',
        '\x24', '\x12', '\x2A', '\x0',  '\x5',  '\xBF', 'G',    '\xD4', '\xEF',
        '\xF7', '\x2F', '6',    '\xEC', '\x12', '\x20', '\x1E', '\x8F', '\xD7',
        '\xAA', '\xD5', '\xEA', '\xAF', 'I',    '5',    'F',    '\xAA', 'T',
        '\x5F', '\x9F', '\x22', 'A',    '\x2A', '\x95', '\xA',  '\x83', '\xE5',
        'r',    '9',    'd',    '\xB3', 'Y',    '\x96', '\x99', 'L',    '\x6',
        '\xE9', 't',    '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',    '\xA7',
        '\xC4', 'b',    '1',    '\xB5', '\x5E', '\x0',  '\x3',  'h',    '\x9A',
        '\xC6', '\x16', '\x82', '\x20', 'X',    'R',    '\x14', 'E',    '6',
        'S',    '\x94', '\xCB', 'e',    'x',    '\xBD', '\x5E', '\xAA', 'U',
        'T',    '\x23', 'L',    '\xC0', '\xE0', '\xE2', '\xC1', '\x8F', '\x0',
        '\x9E', '\xBC', '\x9',  'A',    '\x7C', '\x3E', '\x1F', '\x83', 'D',
        '\x22', '\x11', '\xD5', 'T',    '\x40', '\x3F', '8',    '\x80', 'w',
        '\xE5', '3',    '\x7',  '\xB8', '\x5C', '\x2E', 'H',    '\x92', '\x4',
        '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g',    '\x98', '\xE9',
        '6',    '\x1A', '\xA6', 'g',    '\x15', '\x4',  '\xE3', '\xD7', '\xC8',
        '\xBD', '\x15', '\xE1', 'i',    '\xB7', 'C',    '\xAB', '\xEA', 'x',
        '\x2F', 'j',    'X',    '\x92', '\xBB', '\x18', '\x20', '\x9F', '\xCF',
        '3',    '\xC3', '\xB8', '\xE9', 'N',    '\xA7', '\xD3', 'l',    'J',
        '\x0',  'i',    '6',    '\x7C', '\x8E', '\xE1', '\xFE', 'V',    '\x84',
        '\xE7', '\x3C', '\x9F', 'r',    '\x2B', '\x3A', 'B',    '\x7B', '7',
        'f',    'w',    '\xAE', '\x8E', '\xE',  '\xF3', '\xBD', 'R',    '\xA9',
        'd',    '\x2',  'B',    '\xAF', '\x85', '2',    'f',    'F',    '\xBA',
        '\xC',  '\xD9', '\x9F', '\x1D', '\x9A', 'l',    '\x22', '\xE6', '\xC7',
        '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15', '\x90', '\x7',  '\x93',
        '\xA2', '\x28', '\xA0', 'S',    'j',    '\xB1', '\xB8', '\xDF', '\x29',
        '5',    'C',    '\xE',  '\x3F', 'X',    '\xFC', '\x98', '\xDA', 'y',
        'j',    'P',    '\x40', '\x0',  '\x87', '\xAE', '\x1B', '\x17', 'B',
        '\xB4', '\x3A', '\x3F', '\xBE', 'y',    '\xC7', '\xA',  '\x26', '\xB6',
        '\xEE', '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
        '\xA',  '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X',    '\x0',  '\x27',
        '\xEB', 'n',    'V',    'p',    '\xBC', '\xD6', '\xCB', '\xD6', 'G',
        '\xAB', '\x3D', 'l',    '\x7D', '\xB8', '\xD2', '\xDD', '\xA0', '\x60',
        '\x83', '\xBA', '\xEF', '\x5F', '\xA4', '\xEA', '\xCC', '\x2',  'N',
        '\xAE', '\x5E', 'p',    '\x1A', '\xEC', '\xB3', '\x40', '9',    '\xAC',
        '\xFE', '\xF2', '\x91', '\x89', 'g',    '\x91', '\x85', '\x21', '\xA8',
        '\x87', '\xB7', 'X',    '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N',
        'N',    'b',    't',    '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
        '\xEC', '\x86', '\x2',  'H',    '\x26', '\x93', '\xD0', 'u',    '\x1D',
        '\x7F', '\x9',  '2',    '\x95', '\xBF', '\x1F', '\xDB', '\xD7', 'c',
        '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF', '\x22', 'J',    '\xC3',
        '\x87', '\x0',  '\x3',  '\x0',  'K',    '\xBB', '\xF8', '\xD6', '\x2A',
        'v',    '\x98', 'I',    '\x0',  '\x0',  '\x0',  '\x0',  'I',    'E',
        'N',    'D',    '\xAE', 'B',    '\x60', '\x82',
};


std::string GetTime()
{
    //time_t :整数类型 用来存储从1970年到现在经过了多少秒
    //tm :结构类型 把日期和时间以结构的形式保存，tm 结构的定义如下：
    time_t lt;
    lt=time(NULL);

    struct tm* ptr;
    ptr= localtime(&lt);//转换为当地时间

    char timebuffer[100];
    strftime(timebuffer,100,"%a, %d %b %Y %H:%M:%S ",ptr);

    return std::string(timebuffer);
}

int ReadData(int fd,std::string &read_buffer,bool& is_disconn)
{
    int read_sum=0;
    //因为epoll是ET模式，所以要一次性将数据读完
    while(true)
    {
        /*!
         对非阻塞I/O：
         1.若当前没有数据可读，函数会立即返回-1，同时errno被设置为EAGAIN或EWOULDBLOCK。
           若被系统中断打断，返回值同样为-1,但errno被设置为EINTR。对于被系统中断的情况，
           采取的策略为重新再读一次，因为我们无法判断缓冲区中是否有数据可读。然而，对于
           EAGAIN或EWOULDBLOCK的情况，就直接返回，因为操作系统明确告知了我们当前无数据
           可读。
         2.若当前有数据可读，那么recv函数并不会立即返回，而是开始从内核中将数据拷贝到用
           户区，这是一个同步操作，返回值为这一次函数调用成功拷贝的字节数。所以说，非阻
           塞I/O本质上还是同步的，并不是异步的。
         */

        char buffer[GlobalValue::BufferMaxSize];
        memset(buffer,'\0',sizeof buffer);//最后一个字符是‘\0’
        int ret= recv(fd,buffer,sizeof buffer,0);
        if(ret<0)
        {
            if(errno == EWOULDBLOCK || errno == EAGAIN) return read_sum;

            else if(errno == EINTR) continue;

            else
            {
                Getlogger()->error("fd{} ReadData failed to read data",fd);
                return -1;
                //--------------4/8严重bug（4天），现象：无法正常断开连接
                //客户端断开链接，服务端这边会触发EPOLLIN，EPOLLOUT，EPOLLRDHUP事件，
                // 有些人可能会在服务端关心EPOLLRDHUP事件，触发后关闭套接字，但是这个处理逻辑不是通用的，有些系统（老的linux系统）未必会触发EPOLLRDHUP。
                // 最常用的做法是关心EPOLLIN事件，
                // 然后在read的时候进行处理：
                // 1. read返回0，对方正常调用close关闭链接
                // 2.read返回-1，需要通过errno来判断，如果不是EAGAIN和EINTR，那么就是对方异常断开链接两种情况服务端都要close套接字
                //
                //
                //之前的写法，是读错误了仍然返回已读数据量(return writes_sum)，但是这样就无法在外面调用断开连接，最后将这里改为return -1;
            }
        }
        else if(ret==0)
        {
            Getlogger()->debug("clinet {} has close the connection", fd);
            is_disconn = true;
            break;
        }
        read_buffer += buffer;//bug----没写进read_buffer
        read_sum+=ret;
      //  std::cout<<read_buffer<<std::endl;
    }
    return read_sum;
}

int WriteData(int fd,std::string& buffer,bool& full)
{
    int write_once=0;
    int write_sum=0;
    const char* wait_to_send= nullptr;
    wait_to_send=buffer.c_str();//c_str()后末尾会加一个'\0'

    int num=buffer.size() + 1;

    while(num)
    {
        write_once=send(fd,wait_to_send,num,0);
        if(write_once < 0)
        {
            if(errno == EINTR) continue;//被系统调用打断

            else if(errno == EAGAIN || errno == EWOULDBLOCK)
            {

                full=true;
                return write_sum;
            }
            else
            {
                Getlogger()->error("failed to write data to socket{}  ___  {}", fd,strerror(errno));
                return -1;//否则表示发生了错误，返回-1，和Read同理
            }
        }
        //bug--别写在上面大括号里面去了
        write_sum+=write_once;
        num-=write_once;
        wait_to_send+=write_once;//向后移动
    }

    if(buffer.size()+1 == write_sum) buffer.clear();
    else buffer=buffer.substr(write_sum);

    return write_sum;
}

int Write_to_fd(int fd,const char* content,int length)
{
    const char* pos=content;
    int write_num=0;

    while(write_num<length)
    {
        int write_once= write(fd,pos,length-write_num);
        if(write_once<0)
        {
            if(errno == EINTR) continue;//忽略系统中断
            else if(errno == EAGAIN) return write_num;//缓冲区满
            else{
                Getlogger()->error("write data to filefd {} error: {}", fd, strerror(errno));
                return -1;
            }
        }
        write_num+=write_once;
        pos+=write_once;
    }
    return write_num;
}

int Read_from_fd(int fd,const char* buffer,int length)//read和write最好配套使用，否则会出现奇怪的问题
{
    int read_sum=0;
    const char* pos=buffer;
    while(true)
    {
        int read_once=read(fd,(void *)pos,length-read_sum);
        if(read_once<0)
        {
            if(errno == EINTR) continue;//忽略系统中断
            else if(errno == EAGAIN) return read_sum;//缓冲区满
            else{
                Getlogger()->error("no http read data from filefd {} error: {}", fd, strerror(errno));
                return -1;                               //否则表示发生了错误，返回-1
            }
        }
        read_sum+=read_once;
        pos+=read_once;
        if(read_once==0) return read_sum;
    }
    return read_sum;
}


std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[])
{
    const char* str="p:s:l:";//llf 字符后面有一个冒号表示必须带参数，两个冒号表示参数是可选的
    int res;
    int port,subReactorSize;
    std::string log_path;
    while((res= getopt(argc,argv,str))!=-1)
    {
        switch (res) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                subReactorSize = atoi(optarg);
                break;
            case 'l': {
                std::regex ExpressOfPath(R"(^\.\/\S*)");//llf start with ./
                std::smatch res{};
                std::string path(optarg);
                std::regex_match(path, res, ExpressOfPath);
                if (res.empty()) {
                    std::cout << "illegal log path" << std::endl;
                    return std::nullopt;
                }
                log_path = res[0];
            }
                break;
            default:
                break;
        }
    }
    return std::tuple<int,int,std::string>(port,subReactorSize,log_path);
}

std::shared_ptr<spdlog::logger> Getlogger(std::string path)
{
    try
    {
        // Create basic file logger (not rotated)
        static auto my_logger = spdlog::basic_logger_mt("basic_logger", path);
        return my_logger;
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }

    return {};
}

int BindAndListen(int pot)
{
    if(pot<0 || pot>65535) return -1;
    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        Getlogger()->error("failed to get the listenfd", strerror(errno));
        close(listenfd);
        return -1;
    }

    /*设置地址重用，实现端口复用，一般服务器都需要设置*/
    //llf socket关闭之后，操作系统不会立即收回对端口的控制权，而是要经历一个等待阶段。此时对这个端口绑定就会出错。想要立即进行绑定，就必须先设置SO_REUSEADDR.
    //  或者在关闭socket的时候，使用setsockopt设置SO_REUSEADDR。才会消除等待时间。
    int reuse= 1; //必须用int，和set no_delay一样
    int res= setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof reuse);
    if(res == -1)
    {
        Getlogger()->error("faied to set listenfd SO_REUSEADDR", strerror(errno));
        close(listenfd);
        return -1;
    }

    int ret=0;
    struct sockaddr_in address{};
    bzero(&address,sizeof address);
    address.sin_family=AF_INET;
    /*!
        INADDR_ANY泛指本机的意思。主要是考虑到主机具有多个网卡的情况。
        不管数据从哪个网卡过来，只要是绑定的端口号过来的数据，都可以接收。
     */
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(pot);

    /*绑定地址*/
    ret=bind(listenfd,reinterpret_cast<const sockaddr*>(&address),sizeof address);
    if(ret==-1)
    {
        Getlogger()->error("faied to bind listenfd ", strerror(errno));
        close(listenfd);
        return -1;
    }
    /*!
      从内核2.2版本之后，listen函数的backlog参数表示的是全连接的数量上限。
      所谓全连接，指的是完成了tcp三次握手处于establish状态的连接。也就是
      说，服务器能够同时与backlog个客户端进行tcp握手以建立连接，accept队
      列的长度在任何时候都最多只能为backlog。在5.4版本之后backlog的默认最
      大值为4096(定义在/proc/sys/net/core/somaxconn)。显然，backlog与
      服务器的最大并发连接数量没有直接的关系，只会影响服务器允许同时发起连
      接的客户端的数量。
     */
    ret=listen(listenfd,4096);//listen的第二个参数是什么含义
    if(ret==-1)
    {
        Getlogger()->error("faied to listen listenfd ", strerror(errno));
        close(listenfd);
        return -1;
    }

    return listenfd;
}

int setnonblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    if(old_option==-1)
    {
        std::cout<<strerror(errno)<<std::endl;
        return -1;
    }

    int new_option=old_option | O_NONBLOCK;
    if(fcntl(fd,F_SETFL,new_option)==-1)
    {
        std::cout<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}


