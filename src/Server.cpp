//
// Created by llf on 2022/3/12.
//

#include "Chanel.h"
#include "Server.h"

SERVER::SERVER(int pot, Eventloop* Main_R, Thread_Pool* T_P)
                :port(pot),listen_fd(BindAndListen(port)),
                 server_main_Reactor(Main_R),server_thread_pool(T_P),
                 listen_CH(new Chanel(listen_fd,true))
{
    if(listen_fd == -1) exit(-1);
    setnonblocking(listen_fd);
}