// Created by llf on 2022/3/11.

#include "Timer.h"
#include "EventLoop.h"

EventLoop::EventLoop(bool ismain): is_mainReactor(ismain),epollfd(epoll_create1(EPOLL_CLOEXEC))
                                   ,wheelOFloop(new TimeWheel(12)),stop(true)//初始化时是停止的
{

}

EventLoop::~EventLoop()
{
    stop=true;
    delete wheelOFloop;
    close(epollfd);
}


bool EventLoop::AddChanel(Chanel* CHNL)
{
    int fd=CHNL->Get_fd();
    epoll_event ev{};

    bzero(&ev,sizeof ev);

    ev.data.fd=fd;
    ev.events = CHNL->Get_events() | EPOLLET;//bug!!!!! 得获取chanel的事件再并

    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev)==-1)
    {
        //日志输入添加失败
        return false;
    }

    //成功后，将该chanel添加到池里面
    chanelpool[fd]=std::unique_ptr<Chanel>(CHNL);

    //如果还是连接socket,则将http数据存入池中
    // 并挂靠定时器
    if(CHNL->Get_isconn())
    {
        if(CHNL->Get_holder())
        {
            //httppool[fd]=std::shared_ptr<HttpData>(CHNL->Get_holder());

            //应该把httpdata和timer关联起来
            Timer* res=wheelOFloop->TimeWheel_insert_Timer(GlobalValue::HttpHEADTime,CHNL->Get_holder());
            if(!res)
            {
                Getlogger()->error("insert timer error");
                return false;
            }

            //更改连接数量
            {
                std::unique_lock<std::mutex> locker(NUMmtx);
                NUM_Conn++;
            }
        }
        else Getlogger()->error("fd {} is connfd ,but it not has a holder",CHNL->Get_fd());

    }

    return true;
}

bool EventLoop::MODChanel(Chanel* CHNL,__uint32_t EV)
{
    if(!CHNL->IsEqualToLast())//防止重复修改
    {
        int fd=CHNL->Get_fd();
        epoll_event ev;
        bzero(&ev,sizeof ev);
        ev.data.fd=fd;
        ev.events=EV | EPOLLET;

        if(epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev)==-1)
        {
            //日志输入修改失败
            return false;
        }

        CHNL->Set_events(EV);
        return true;
    }
    return false;
}

bool EventLoop::DELChanel(Chanel* CHNL)
{
    int fd=CHNL->Get_fd();
    epoll_event ev;
    bzero(&ev,sizeof ev);
    ev.data.fd=fd;
    ev.events=0;

    if(epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev)==-1)
    {
        //日志输入删除失败
        Getlogger()->error("failed to delete fd in epoll");
        return false;
    }

    if(CHNL->Get_holder())
    {
        //删除http池中的数据和定时器
        get_theTimeWheel()->TimerWheel_Remove_Timer(CHNL->Get_holder()->Get_timer());
      //  httppool[fd]=nullptr;
    }
    chanelpool[fd].reset(nullptr);
    return true;

}

void EventLoop::StartLoop()
{
    stop= false;
    while(!stop)
    {
        ListenAndCall();
    }
    stop=true;
}

void EventLoop::ListenAndCall()
{
    while(!stop)
    {
        int number= epoll_wait(epollfd,events,PerEpollMaxEvent,Epoll_timeout);
        if(number<0&& errno !=EINTR)
        {
            //日志输出epoll失败
            Getlogger()->debug("System call interrupts epoll");
            //这里不对系统调用做处理
            continue;
        }
        if(number==0)
        {
            //日志输出epoll超时
            Getlogger()->debug("epoll timeout");
            continue;
        }
        for(int i=0;i<number;++i)
        {
            int sockfd=events[i].data.fd;

            Chanel* chanelnow=chanelpool[sockfd].get();
            if(chanelnow)
            {
                chanelnow->Set_revents(events[i].events);
                chanelnow->CallRevents();
            }
            else
            {
                //日志输出：曾有添加chanel到事件池中失败的历史
                Getlogger()->debug("not get the correct chanel");
            }
        }
    }
}

void EventLoop::StopLoop()
{
    stop=true;
}

int EventLoop::Get_Num_Conn()
{
    int num=0;
    {
        std::unique_lock<std::mutex> locker(NUMmtx);
        num=NUM_Conn;
    }
    return num;
}