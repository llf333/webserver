// Created by llf on 2022/3/12.

#include <netinet/in.h>
#include <netinet/tcp.h>
#include "Server.h"


SERVER* SERVER::service= nullptr;//定义必须写在.cpp文件中
std::mutex SERVER::init_lock{};

//2022-3-14
SERVER::SERVER(int pot, EventLoop* Main_R, Thread_Pool* T_P)
                :port(pot),listen_fd(BindAndListen(port)),
                 server_main_Reactor(Main_R),server_thread_pool(T_P),
                 listen_CH(new Chanel(listen_fd,false))//4/5监听socket不是连接socket
{
    if(listen_fd == -1) exit(-1);
    setnonblocking(listen_fd);
}

SERVER:: ~SERVER()
{
    //llf 4/6 资源管理整理：
    /*！
     * 资源管理的原则是需要留意带指针的对象
     * static SERVER* service              在主函数分配，由主函数管理
     * Chanel* listen_CH                   Chanel通常由Reactor管理，有两种途径，分别是：Reactor中DELChanel函数删除，以及 Reactor析构时自动删除（采用了unique_ptr的数组）
     *                                     此处交由mainReactor管理
     * Thread_Pool* server_thread_pool     在主函数分配，由主函数管理
     * SubReactors                         使用智能指针在析构时自动管理子Reactor
     */


}

void SERVER::Server_Start()
{
    /*对监听socket监听可读以及异常事件*/
    listen_CH->Set_events(EPOLLIN | EPOLLERR);
    listen_CH->Register_ErHandle([=]{ERRisComing();});
    listen_CH->Register_RdHandle([=] {CONNisComing();});

    server_main_Reactor->AddChanel(listen_CH);

    //给线程池中的任务队列添加任务，任务是跑子Reactor，每个线程对应一个子Reactor
    auto subreactor_size=server_thread_pool->sizeofpoll;
    for(decltype(subreactor_size) i =0;i<subreactor_size;++i)//llf 线程池有多大，开几个subReactor
    {
        /*HttpServer和ThreadPool需要共享SubReactor对象，故这里使用shared_ptr*/
        std::shared_ptr<EventLoop> sub(new EventLoop(false));
        if(!sub)
        {
            Getlogger()->warn("fail to creat a subreactor", strerror(errno));
            continue;
        }
        SubReactors.emplace_back(sub);//llf 记录子Reactor

        //??????llf 在线程池中添加任务（任务是跑子Reactor），等号表示值捕获，函数返回结果存在哪儿？————future中包装的是Void，所以不用获取
        server_thread_pool->Add_task([=]{sub->StartLoop();});//每个线程的任务是跑SubReatcor
        timeWheel_PipeOfWrite.emplace_back(sub->get_theTimeWheel()->Get_1tick());//保存每个时间轮的tick管道写端，往里面写数据意味着tick一下
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
    while(true)//ET模式，每次都要接受完，因此使用while循环将accept包住，保证一个可读事件来临，处理完全部的连接请求
    {
        //参考https://www.bianchengquan.com/article/563552.html
        int connfd=accept(listen_fd,reinterpret_cast<struct sockaddr*>(&client_address),&client_addrlen);

        if(connfd<0)
        {
            if(errno!=EAGAIN && errno!=EWOULDBLOCK)
            {
                close(connfd);
                Getlogger()->error("failed to accept", strerror(errno));
            }
            return ;//bug----return应该写在外面
        }

        //限制服务器的最大并发连接数
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

        //bug 设置该选项的enable必须使用int，char类型不行，每个控制选项的类型不同，查表而知。
        const int enable=1;
        int ret=setsockopt(connfd,IPPROTO_TCP,TCP_NODELAY,&enable,sizeof (int));


        if(ret==-1)
        {
            std::cout<<strerror(errno)<<std::endl;
            ::Getlogger()->error("failed to set TCP_NODELAY on connfd", strerror(errno));
            close(connfd);
            return ;
        }



        //建立好新连接之后，把任务派发给子Reactor（派发形式是找连接数量最少的子Reactor）
        //注意这里还没加入子Reactor的httpdata池中，也没设置holder————已经在addchanel中加入了

        Chanel* newconn(new Chanel(connfd, true));

        /*!
            Http连接的sokcet需要监听可读、可写、断开连接以及错误事件。
            但是需要注意的是，不要一开始就注册可写事件，因为只要fd只要不是阻塞的它就是可写的。
            因此，需要在完整读取了客户端的数据之后再注册可写事件，否则会一直触发可写事件。
            这里connfd_channel的生命周期交由SubReactor管理。
         */
        newconn->Set_events(EPOLLIN | EPOLLRDHUP | EPOLLERR);

        //将连接socket分发给事件最少的SubReactor
        int least_conn_num=SubReactors[0]->Get_Num_Conn(),idx=0;
        for(int i=0;i<SubReactors.size();++i)
        {
            if(SubReactors[i]->Get_Num_Conn()<least_conn_num)
            {
                least_conn_num=SubReactors[i]->Get_Num_Conn();
                idx=i;
            }
        }

        //必须先设置Holder再将该连接socket加入到事件池中
        HttpData* newholder=new HttpData(newconn,SubReactors[idx].get());//chanel的四个回调函数在这里面绑定
        newconn->Set_holder(newholder);

        //分发
        SubReactors[idx]->AddChanel(newconn);

        Getlogger()->info("SubReactor {} add a connect :{}, CurrentUserNumber: {}",idx,connfd,GlobalValue::GetUserNUmber());

       // std::cout<<"get a new conn222"<<std::endl;
    }
}

