// Created by llf on 2022/3/11.
#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include<sys/epoll.h>
#include<memory>
#include "Timer.h"
#include "Other.h"


class HttpData;
class Chanel;

//llf 一个事件驱动循环EventLoop其实就是一个Reactor模型，是一个单线程任务，一个线程负责一个io复用。主要包含io复用函数epoll，时间轮。其他的就是一些辅助变量
class EventLoop
{
private:
    bool is_mainReactor=false;

    int NUM_Conn;                                                                   //Reactor管理的http连接的数量
    std::mutex NUMmtx;                                                              //避免访问 "连接数量" 造成竞争

    static const int PerEpollMaxEvent=4096;
    static const int Epoll_timeout=10000;                                               //epoll超时时间10秒(单位为毫秒)，如果设置为0，理论上很占cpu，因为内核会一直调用epoll，但还未测试

    int epollfd;
    epoll_event events[PerEpollMaxEvent];                                           //epoll在使用时要配一个epoll_event数组

    Chanel* chanelpool[GlobalValue::TheMaxConnNumber];                           //事件池 //llf 中括号的优先级比较高，因此是个数组，元素类型为指针
    //为什么要设置这个数组，原因是使得在监听时可以直接根据fd值来访问对应Chanel,相当于起一个映射的关系；04/06——还有资源管理的目的

    HttpData* httppool[GlobalValue::TheMaxConnNumber];
   //感觉可以删除，好像没必要存这个映射，可以用Chanel->GetHolder-----22/04/05原版本的目的是用于资源管理


    TimeWheel wheelOFloop{};                                                         //为了避免竞争，让每个事件池都拥有一个独立的时间轮

    bool stop;                                                                      //指示Sub/Main-Reactor是否工作，默认为停止

public:
    explicit EventLoop(bool ismain);
    ~EventLoop();

    bool AddChanel(Chanel* CHNL);
    bool MODChanel(Chanel* CHNL,__uint32_t EV);
    bool DELChanel(Chanel* CHNL);
    int Get_Num_Conn();


    void StartLoop();
    void StopLoop();

    TimeWheel& get_theTimeWheel(){return wheelOFloop;}

private:
    void ListenAndCall();
};

#endif //WEBSERVER_EVENTLOOP_H
