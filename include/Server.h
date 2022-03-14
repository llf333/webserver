// Created by llf on 2022/3/12.

#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include<vector>
#include<memory>

#include "ThreadPool.h"
#include "Other.h"

class Chanel;
class Eventloop;

class SERVER
{
private:
    static SERVER* service;//使用单例饿汉模式，注意全局只能初始化一次，饿汉模式线程不安全

    int port;//这里刻意地调整了一下顺序以防出错
    int listen_fd;
    Chanel* listen_CH;

    Eventloop* server_main_Reactor;
    std::vector<std::shared_ptr<Eventloop>> SubReactors;//这里使用智能指针在析构时自动管理子Reactor
    Thread_Pool* server_thread_pool;

    SERVER(int pot, Eventloop* mainreactor, Thread_Pool* T_P);
    ~SERVER();

public:
    std::vector<int> timeWheel_PipeOfWrite{};

public:
    void Server_Start();
    void Server_Stop();

    SERVER* Get_the_service(int pot, Eventloop* Main_R, Thread_Pool* T_P)
    {
        if(service!= nullptr) return service;
        else
        {
            service=new SERVER (pot, Main_R,T_P);
            return service;
        }
    }
};

SERVER* SERVER::service= nullptr;

#endif //WEBSERVER_SERVER_H
