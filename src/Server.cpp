// Created by llf on 2022/3/12.

#include <netinet/in.h>
#include <netinet/tcp.h>
#include "Server.h"


SERVER* SERVER::service= nullptr;//定义必须写在.cpp文件中
//2022-3-14
SERVER::SERVER(int pot, EventLoop* Main_R, Thread_Pool* T_P)
                :port(pot),listen_fd(BindAndListen(port)),
                 server_main_Reactor(Main_R),server_thread_pool(T_P),
                 listen_CH(new Chanel(listen_fd,true))
{
    if(listen_fd == -1) exit(-1);
    setnonblocking(listen_fd);
}

SERVER:: ~SERVER()
{
    //资源管理还未整理
}

void SERVER::Server_Start()
{
    listen_CH->Set_events(EPOLLIN | EPOLLERR);//监听socket监听可读事件和异常事件
    listen_CH->Register_ErHandle([=]{ERRisComing();});
    listen_CH->Register_RdHandle([=] {CONNisComing();});

    server_main_Reactor->AddChanel(listen_CH);

    auto subreactor_size=server_thread_pool->sizeofpoll;
    for(decltype(subreactor_size) i =0;i<subreactor_size;++i)
    {
        std::shared_ptr<EventLoop> sub(new EventLoop(false));
        if(!sub)
        {
            Getlogger()->warn("fail to creat a subreactor", strerror(errno));
            continue;
        }
        SubReactors.emplace_back(sub);
        server_thread_pool->Add_task([=]{sub->StartLoop();});//每个线程的任务是跑SubReatcor
        timeWheel_PipeOfWrite.emplace_back(sub->get_theTimeWheel()->tick_d[1]);
    }
}

void SERVER::Server_Stop()
{
    close(listen_fd);
    server_main_Reactor->StopLoop();
    for(auto it:SubReactors)
    {
        it->StopLoop();
    }
}

void SERVER::ERRisComing()
{
    Getlogger()->error("SERVER accpet a error ", strerror(errno));
}


void SERVER::CONNisComing()
{
    struct sockaddr_in client_address;
    socklen_t client_addrlen=sizeof(client_address);
    while(true)
    {
        int connfd=accept(listen_fd,reinterpret_cast<struct sockaddr*>(&client_address),&client_addrlen);

        if(connfd<0)
        {
            if(errno!=EAGAIN && errno!=EWOULDBLOCK)
            {
                close(connfd);
                Getlogger()->error("failed to accept", strerror(errno));
                return ;
            }
        }

        if(GlobalValue::CurrentUserNumber>=GlobalValue::TheMaxConnNumber)
        {
            close(connfd);
            Getlogger()->warn("too many connect");
            return ;
        }
        GlobalValue::Inc_Current_user_number();

        if(setnonblocking(connfd) < 0)
        {
            close(connfd);
            Getlogger()->error("failed to set nonblocking on connfd");
            return;
        }
        /*!
         *llf
         * TCP/IP协议中针对TCP默认开启了Nagle算法。Nagle算法通过减少需要传输的数据包，来优化网络。在内核实现中，数据包的发送和接受会先做缓存，分别对应于写缓存和读缓存。
         启动TCP_NODELAY，就意味着禁用了Nagle算法，允许小包的发送。对于延时敏感型，同时数据传输量比较小的应用，开启TCP_NODELAY选项无疑是一个正确的选择。
         比如，对于SSH会话，用户在远程敲击键盘发出指令的速度相对于网络带宽能力来说，绝对不是在一个量级上的，所以数据传输非常少；
         而又要求用户的输入能够及时获得返回，有较低的延时。如果开启了Nagle算法，就很可能出现频繁的延时，导致用户体验极差。
         当然，你也可以选择在应用层进行buffer，比如使用java中的buffered stream，尽可能地将大包写入到内核的写缓存进行发送；
         vectored I/O（writev接口）也是个不错的选择。
         */

        const char enable=1;
        int ret=setsockopt(connfd,IPPROTO_TCP,TCP_NODELAY,&enable,sizeof enable);
        if(ret==-1)
        {
            close(connfd);
            Getlogger()->error("failed to set TCP_NODELAY on connfd", strerror(errno));
            return ;
        }


        //建立好新连接之后，把任务派发给子Reactor（派发形式是找连接数量最少的子Reactor）
        //注意这里还没加入子Reactor的httpdata池中，也没设置holder——已经在addchanel中加入了

        Chanel* newconn(new Chanel(connfd, true));
        /*!
            Http连接的sokcet需要监听可读、可写、断开连接以及错误事件。
            但是需要注意的是，不要一开始就注册可写事件，因为只要fd只要不是阻塞的它就是可写的。
            因此，需要在完整读取了客户端的数据之后再注册可写事件，否则会一直触发可写事件。
            这里connfd_channel的生命周期交由SubReactor管理。
         */
        newconn->Set_events(EPOLLIN | EPOLLRDHUP | EPOLLERR);


        int least_conn_num=SubReactors[0]->Get_Num_Conn(),idx=0;
        for(int i=0;i<SubReactors.size();++i)
        {
            if(SubReactors[i]->Get_Num_Conn()<least_conn_num)
            {
                least_conn_num=SubReactors[i]->Get_Num_Conn();
                idx=i;
            }
        }

        HttpData* newholder=new HttpData(newconn,SubReactors[idx].get());
        newconn->Set_holder(newholder);

        SubReactors[idx]->AddChanel(newconn);

        Getlogger()->info("SubReactor {} add a connect :{}",idx,connfd);
    }
}

