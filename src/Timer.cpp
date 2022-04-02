// Created by llf on 2022/3/7.

#include <unistd.h>
#include "Timer.h"


//****************定时器*********************//

Timer::Timer(size_t pos,size_t turns):PosInWheel(pos),Turns(turns)
{

}

void Timer::ExecuteCallbackFunc()
{
    if(FuncOfTimeUp) FuncOfTimeUp();
    else Getlogger()->debug("timer callbackFunc is not registered");
}



//****************时间轮*********************//

TimeWheel::TimeWheel(size_t maxsize):SizeOfTW(maxsize),Si(GlobalValue::TimeWheel_PerSlotTime)
{
    slot=std::vector< std::list<Timer*> >(SizeOfTW,std::list<Timer*>());

    int res= pipe(tick_d);
    if(res==-1)
    {
        Getlogger()->error("TimeWheel create pipe error :{}", strerror(errno));
        exit(-1);
    }
    setnonblocking(tick_d[0]);
    setnonblocking(tick_d[1]);

    tick_chanel=new Chanel(tick_d[0],false);
    tick_chanel->Set_events(EPOLLIN);
    tick_chanel->Register_RdHandle([=]{tick();});
}
TimeWheel::~TimeWheel()
{
    for(auto& l:slot)
        for(auto& timer:l)
            delete timer;

    close(tick_d[0]);
    close(tick_d[1]);
}

//新建时间器在该函数里，且和holder绑定
Timer* TimeWheel::TimeWheel_insert_Timer(std::chrono::seconds timeout,HttpData* holder)
{
    if(timeout<std::chrono::seconds(0)) return nullptr;

    size_t pos=0,cycle=0;
    if(timeout<Si)    cycle=0,pos=1;                           //向上整合
    else
    {
        cycle=(timeout/Si)/SizeOfTW;
        pos=(CurrentPos+(timeout/Si)%SizeOfTW)%SizeOfTW;
    }
    Timer *Temp=nullptr;
    Temp = new Timer(pos, cycle);
    slot[pos].push_back(Temp);

    //http事件挂靠定时器,并设置超时回调函数
    if(holder)
    {
        //相互绑定
        Temp->Register_CallbackFunc([=]{holder->TimerTimeoutCallback();});
        holder->Set_timer(Temp);
    }

    return Temp;
}

bool TimeWheel::TimerWheel_Remove_Timer(Timer* timer)
{
    if(!timer) return false;
    slot[timer->PosInWheel].remove(timer);
    delete timer;
    return true;
}

bool TimeWheel::TimerWheel_Adjust_Timer(Timer* timer,std::chrono::seconds timeout)
{
    if(!timer) return false;
    if(timeout<std::chrono::seconds(0)) return false;

    size_t pos=0,cycle=0;
    if(timeout<Si)    cycle=0,pos=1;                           //向上整合
    else
    {
        cycle=(timeout/Si)/SizeOfTW;
        pos=(CurrentPos+(timeout/Si)%SizeOfTW)%SizeOfTW;
    }

    for(auto& it:slot[timer->PosInWheel])
    {
        if(it==timer){
            it->PosInWheel=pos;
            it->Turns=cycle;
            return true;
        }
    }
    return false;
}

//2022/03/12
void TimeWheel::tick()
{
    //先把管道里的数据读了，避免堵住了
    std::string buffer;
    bool dis_conn=false;
    int ret=ReadData(tick_d[0],buffer,dis_conn);
    if(ret<=0)
    {
        //打印失败日志
        Getlogger()->error("in tick ,read data to buffer error", strerror(errno));
        return ;
    }

    for(auto it=slot[CurrentPos].begin();it!=slot[CurrentPos].end();)
    {
        Timer* p=*it;//防止删除空指针
        if(!p)
        {
            slot[CurrentPos].erase(it);//erase删除后已经指向了下一个元素
            continue;
        }

        if(p->Timer_GetTurns()>0)
        {
            p->Timer_TurnsDecline();
            ++it;
            continue;
        }
        if(p->Timer_GetTurns()==0)
        {
            p->ExecuteCallbackFunc();
            slot[CurrentPos].erase(it);
        }
    }

    CurrentPos=(CurrentPos+1)%SizeOfTW;
}












