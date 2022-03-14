// Created by llf on 2022/3/11.

#include "Timer.h"
#include "EventLoop.h"

EventLoop::EventLoop(bool ismain): is_mainReactor(ismain),epollfd(epoll_create1(EPOLL_CLOEXEC))
                                   ,wheelOFloop(new TimeWheel(12)),stop(true)
{

}

EventLoop::~EventLoop()
{
    stop=false;
    delete wheelOFloop;
    close(epollfd);
}


bool EventLoop::AddChanel(Chanel* CHNL,__uint32_t EV)
{
    int fd=CHNL->Get_fd();
    epoll_event ev;

    bzero(&ev,sizeof ev);

    ev.data.fd=fd;
    ev.events=EV | EPOLLET;

    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev)==-1)
    {
        //日志输入添加失败
        return false;
    }

    //成功后，将该chanel添加到池里面
    chanelpool[fd]=std::unique_ptr<Chanel>(CHNL);

    //如果还是连接socket,则将http数据存入池中，
    // 并挂靠定时器
    if(CHNL->Get_isconn())
    {
      //  httppool[fd]=std::unique_ptr<Httpdata>(CHNL->Get_holder());
        //暂时还没挂靠定时器

        {
            std::unique_lock<std::mutex> locker(NUMmtx);
            NUM_Conn++;
        }
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
        return false;
    }

    if(CHNL->Get_holder())
    {
        //删除http池中的数据和定时器
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
            //这里不对系统调用做处理
        }
        if(number==0)
        {
            //日志输出epoll超时
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
            }
        }
    }
}

void EventLoop::StopLoop()
{
    stop=true;
}