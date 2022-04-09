// Created by llf on 2022/3/11.

#ifndef WEBSERVER_CHANEL_H
#define WEBSERVER_CHANEL_H

#include<stdint.h>
#include<functional>
#include "Other.h"
#include "EventLoop.h"
#include "HttpData.h"

class HttpData;

class Chanel{
    //Chanel在 void SERVER::CONNisComing() 中新建，每个时间轮监听tick也有一个，listen_chanel也有一个

private:
    int fd;
    bool is_connect;
    __uint32_t events;//感兴趣的事件集
    __uint32_t revents;//已就绪的事件集

    HttpData* holder=nullptr;//什么时候删除的--在delete Chanel中删除-----------指针最好设置默认值

    using CALLBACK=std::function<void()>;
    CALLBACK read_handle;
    CALLBACK write_hande;
    CALLBACK error_handle;
    CALLBACK disconn_handle;

public:
   // Chanel(int _fd, bool is_conn_, std::chrono::seconds timeout=GlobalValue::client_header_timeout);
    explicit Chanel(int _fd, bool is_conn_);
    ~Chanel();
    void Register_RdHandle(CALLBACK func) {read_handle=std::move(func);}
    void Register_WrHandle(CALLBACK func) {write_hande=std::move(func);}
    void Register_ErHandle(CALLBACK func) {error_handle=std::move(func);}
    void Register_DiscHandle(CALLBACK func) {disconn_handle=std::move(func);}
    void CallRevents();

    __uint32_t Get_events(){return events;}

    //设置感兴趣事件
    void Set_events(__uint32_t evnt) {events=evnt;}

    //设置已就绪事件（根据epoll_wait的结果设置）
    void Set_revents(__uint32_t revnt) {revents=revnt;}

    int Get_fd() {return fd;}

    HttpData* Get_holder(){return holder;}

    void Set_holder(HttpData* holder_){holder=holder_;}

    bool Get_isconn(){return is_connect;};


private:
    void CallRdfunc();
    void CallWrfunc();
    void CallErfunc();
    void CallDiscfunc();

};

#endif //WEBSERVER_CHANEL_H
