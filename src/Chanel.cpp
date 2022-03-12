// Created by llf on 2022/3/11.

#include "Chanel.h"

Chanel::Chanel(int _fd, bool is_conn_, std::chrono::seconds timeout): fd(_fd),is_connect(is_conn_)
{
}

Chanel::~Chanel()
{
    close(fd);
}

void Chanel::CallRevents()
{
    if(this->revents & EPOLLERR)
    {
        CallErfunc();
        return ;
    }
    if(this->revents & EPOLLIN)
        CallRdfunc();
    if(this->revents & EPOLLOUT)
        CallWrfunc();
    if(this->revents & EPOLLRDHUP)
        CallDiscfunc();
}

void Chanel::CallRdfunc()
{
    if(read_handle) read_handle();
    else;//打印日志：还没注册
}

void Chanel::CallWrfunc()
{
    if(write_hande) write_hande();
    else ;//打印日志：还没注册
}
void Chanel::CallErfunc()
{
    if(error_handle) error_handle();
    else;//打印日志：还没注册
}

void Chanel::CallDiscfunc()
{
    if(disconn_handle) disconn_handle();
    else;//打印日志：还没注册
}

bool Chanel::IsEqualToLast()
{
    __uint32_t tmp=last_event;
    last_event=Get_events();
    return tmp==Get_events();
}
