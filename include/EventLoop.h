// Created by llf on 2022/3/11.
#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include<sys/epoll.h>
#include<memory>
#include<Other.h>
#include<Chanel.h>

class Httpdata;

class EventLoop
{
private:
    bool is_mainReactor;

    int NUM_Conn;
    std::mutex NUMmtx;

    int epollfd;
    static const int PerEpollMaxEvent=4096;
    epoll_event events[PerEpollMaxEvent];
    std::unique_ptr<Chanel> chanelpool[PerEpollMaxEvent];
    std::unique_ptr<Httpdata> httppool[PerEpollMaxEvent];

    TimeWheel wheelOFloop;

public:
    bool AddChanel(Chanel* CHNL,__uint32_t EV);
    bool MODChanel(Chanel* CHNL,__uint32_t EV);
    bool DELChanel(Chanel* CHNL);


    void StartLoop();
    void StopLoop();

};

#endif //WEBSERVER_EVENTLOOP_H
