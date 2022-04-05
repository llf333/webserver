// Created by llf on 2022/3/11.

#include "Timer.h"
#include "EventLoop.h"

EventLoop::EventLoop(bool ismain): is_mainReactor(ismain),epollfd(epoll_create1(EPOLL_CLOEXEC))
                                   ,wheelOFloop(new TimeWheel(12)),stop(true)//初始化时是停止的
{
    if(!AddChanel(wheelOFloop->Get_tickChanel()) || epollfd==-1)//将该Reactor的时间轮的“监听tick[0]的事件”添加到事件池中
    {
        Getlogger()->error("create reactor erro : {}", strerror(errno));
        exit(-1);
    }

}

EventLoop::~EventLoop()
{
    stop=true;
    delete wheelOFloop;
    close(epollfd);
}

//将要监听的事件添加到epoll中
bool EventLoop::AddChanel(Chanel* CHNL)
{
    int fd=CHNL->Get_fd();
    epoll_event ev{};

    //bzero(&ev,sizeof ev);//bzero是linux特有的，推荐使用memset
    memset(&ev,0,sizeof ev);

    ev.data.fd=fd;
    ev.events = CHNL->Get_events() | EPOLLET;//bug!!!!! 得获取chanel的事件再并

    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev)==-1)
    {
        //日志输入添加失败
        Getlogger()->error("add chanel to epoll fail {}", strerror(errno));
        return false;
    }

    //成功后，将该chanel添加到池里面
    chanelpool[fd]=std::unique_ptr<Chanel>(CHNL);//又新建一个指针————注意资源管理——————————4/5好像是错的，这句话的意思是将指针用智能指针管理

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
            Getlogger()->error("modify chanel in epoll fail {}", strerror(errno));
            return false;
        }

        CHNL->Set_events(EV);
        return true;
    }
    return false;
}

bool EventLoop::DELChanel(Chanel* CHNL)//定时器也在这里面删除
{
    if(!CHNL) return false;

    int fd=CHNL->Get_fd();
    epoll_event ev;
    //推荐使用memset//bzero(&ev,sizeof ev);
    memset(&ev,0,sizeof ev);
    ev.data.fd=fd;
    ev.events=0;

    if(epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev)==-1)
    {
        //日志输入删除失败
        Getlogger()->error("failed to delete fd {} in epoll :   {}", fd,strerror((errno)));
        if(CHNL->Get_holder()) std::cout<<fd<<" has holder"<<std::endl;
        return false;
    }

    if(CHNL->Get_holder())
    {
        //删除http池中的数据和定时器
        if(CHNL->Get_holder()->Get_timer())
        get_theTimeWheel()->TimerWheel_Remove_Timer(CHNL->Get_holder()->Get_timer());
      //  httppool[fd]=nullptr;
        if(CHNL->Get_holder())
        delete CHNL->Get_holder();  //4/5，重大bug————长时间没判断出为什么会有莫名奇妙的fd值（特别大或者是负数），log中总是显示删除fd失败，写数据失败。
                                    //怀疑是内存泄漏，于是从头到尾地明确了一下各个对象是什么时候被delete的，原版httpdata是用unique_ptr管理的，
                                    // 但是我觉得在eventloop中存储httpdata的没有必要，因此此处需要手动删除。
    }

    chanelpool[fd].reset(nullptr);//删除原始指针，将unique_ptr管理至nullptr

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