// Created by llf on 2022/3/11.
#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include<sys/epoll.h>
#include<memory>
#include "Other.h"
#include "Chanel.h"
#include "Timer.h"

class HttpData;
class Chanel;
class TimeWheel;

class EventLoop
{
private:
    bool is_mainReactor;

    int NUM_Conn;
    std::mutex NUMmtx;

    int epollfd;
    static const int PerEpollMaxEvent=4096;
    static const int Epoll_timeout=1;//如果设置为0，理论上很占cpu，因为内核会一直调用epoll，但还未测试
    epoll_event events[PerEpollMaxEvent];
    std::unique_ptr<Chanel> chanelpool[PerEpollMaxEvent];//为什么要设置这个数组，原因是使得在监听时可以直接根据fd值来访问对应Chanel,相当于起一个映射的关系
   // std::shared_ptr<HttpData> httppool[PerEpollMaxEvent];//感觉可以删除，好像没必要存这个映射，可以用Chanel->GetHolder

    TimeWheel* wheelOFloop;

    bool stop;

public:
    EventLoop(bool ismain);
    ~EventLoop();
    bool AddChanel(Chanel* CHNL);
    bool MODChanel(Chanel* CHNL,__uint32_t EV);
    bool DELChanel(Chanel* CHNL);
    int Get_Num_Conn();


    void StartLoop();
    void StopLoop();

    TimeWheel* get_theTimeWheel(){return wheelOFloop;}

private:
    void ListenAndCall();
};

#endif //WEBSERVER_EVENTLOOP_H
