// Created by llf on 2022/3/11.

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "Other.h"

///////////////
//Global Value//
///////////////

std::chrono::seconds GlobalValue::client_header_timeout=std::chrono::seconds(60);
int GlobalValue::TheMaxConnNumber=100000;
int GlobalValue::CurrentUserNumber=0;


bool ReadData(int fd,char* buffer);


std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[])
{
    const char* str="p:s:l:";
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

    //设置地址重用
    bool reuse=true;
    int res= setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<void *>(reuse),sizeof reuse);
    if(!res)
    {
        Getlogger()->error("faied to set listenfd SO_REUSEADDR", strerror(errno));
        close(listenfd);
        return -1;
    }

    int ret=0;
    struct sockaddr_in address{};
    bzero(&address,sizeof address);
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(pot);

    ret=bind(listenfd,reinterpret_cast<const sockaddr*>(&address),sizeof address);
    if(ret==-1)
    {
        Getlogger()->error("faied to bind listenfd ", strerror(errno));
        close(listenfd);
        return -1;
    }

    ret=listen(listenfd,5000);
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
    if(old_option==-1) return -1;

    int new_option=old_option | O_NONBLOCK;
    if(fcntl(fd,F_SETFL,new_option)==-1) return -1;

    return 0;
}

bool ReadData(int fd,char* buffer)
{
    return true;
}
