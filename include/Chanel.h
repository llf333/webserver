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
private:
    int fd;
    bool is_connect;
    __uint32_t events;//感兴趣的事件集
    __uint32_t revents;//已就绪的事件集
    __uint32_t last_event;//记录上一次的防止重复修改

    HttpData* holder;

    using CALLBACK=std::function<void()>;
    CALLBACK read_handle;
    CALLBACK write_hande;
    CALLBACK error_handle;
    CALLBACK disconn_handle;

public:
    Chanel(int _fd, bool is_conn_, std::chrono::seconds timeout=GlobalValue::client_header_timeout);
    ~Chanel();
    void Register_RdHandle(CALLBACK func) {read_handle=std::move(func);}
    void Register_WrHandle(CALLBACK func) {write_hande=std::move(func);}
    void Register_ErHandle(CALLBACK func) {error_handle=std::move(func);}
    void Register_DiscHandle(CALLBACK func) {disconn_handle=std::move(func);}
    void CallRevents();

    __uint32_t Get_events(){return events;}
    void Set_events(__uint32_t evnt) {events=evnt;}

    void Set_revents(__uint32_t revnt) {revents=revnt;}

    int Get_fd() {return fd;}

    HttpData* Get_holder(){return holder;}
    void Set_holder(HttpData* holder_){holder=holder_;}

    bool Get_isconn(){return is_connect;};
    bool IsEqualToLast();

private:
    void CallRdfunc();
    void CallWrfunc();
    void CallErfunc();
    void CallDiscfunc();

};

#endif //WEBSERVER_CHANEL_H
