// Created by llf on 2022/3/12.

#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include<vector>
#include<memory>

#include "ThreadPool.h"
#include "Other.h"
#include "Chanel.h"
#include "EventLoop.h"
#include "HttpData.h"

class EventLoop;
class Chanel;
class Thread_Pool;
class HttpData;



class SERVER
{
private:
    static SERVER* service;//使用单例饿汉模式，注意全局只能初始化一次，饿汉模式线程不安全
    SERVER(int pot, EventLoop* mainreactor, Thread_Pool* T_P);

    int port;//这里刻意地调整了一下顺序以防出错
    int listen_fd;
    Chanel* listen_CH;

    EventLoop* server_main_Reactor;
    std::vector<std::shared_ptr<EventLoop>> SubReactors;//这里使用智能指针在析构时自动管理子Reactor
    Thread_Pool* server_thread_pool;

public:
    std::vector<int> timeWheel_PipeOfWrite{};
public:
    void Server_Start();
    void Server_Stop();
    ~SERVER();

    static SERVER* Get_the_service(int pot, EventLoop* Main_R, Thread_Pool* T_P)
    {
        if(service!= nullptr) return service;
        else
        {
            service=new SERVER (pot, Main_R,T_P);
            return service;
        }
    }

private:
    void ERRisComing();
    void CONNisComing();
};



#endif //WEBSERVER_SERVER_H
