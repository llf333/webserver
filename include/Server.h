// Created by llf on 2022/3/12.

#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include<vector>
#include<memory>

class Chanel;
class Eventloop;

class SERVER
{
private:
    static SERVER* service;//使用单例饿汉模式，注意全局只能初始化一次，饿汉模式线程不安全
    int listen_fd;
    int port;
    Chanel* listen_CH;

    Eventloop* mainReactor;
    std::vector<std::shared_ptr<Eventloop>> SubReactors;//这里使用智能指针在析构时自动管理子Reactor

    SERVER();
    ~SERVER();

public:
    void Server_Start();
    void Server_Stop();

    SERVER* Get_the_service()
    {
        if(service!= nullptr) return service;
        else
        {
            service=new SERVER ();
            return service;
        }
    }
};

SERVER* SERVER::service= nullptr;

#endif //WEBSERVER_SERVER_H
