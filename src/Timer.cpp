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
}



//****************时间轮*********************//

TimeWheel::TimeWheel(size_t maxsize):SizeOfTW(maxsize),Si(1)
{
    slot=std::vector< std::list<Timer*> >(SizeOfTW,std::list<Timer*>());

}
TimeWheel::~TimeWheel()
{
    for(auto& l:slot)
        for(auto& timer:l)
            delete timer;

    close(tick_d[0]);
    close(tick_d[1]);
}

Timer* TimeWheel::TimeWheel_insert_Timer(std::chrono::seconds timeout)
{
    if(timeout<std::chrono::seconds(0)) return nullptr;

    size_t pos=0,cycle=0;
    if(timeout<Si)    cycle=0,pos=1;                           //向上整合
    else
    {
        cycle=(timeout/Si)/SizeOfTW;
        pos=(CurrentPos+(timeout/Si)%SizeOfTW)%SizeOfTW;
    }
    Timer *Temp;
    Temp = new Timer(pos, cycle);
    slot[pos].push_back(Temp);
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
    //先把管道里的数据读了，堵住了
    char buffer[1024];
    bool ret=ReadData(tick_d[0],buffer);
    if(!ret)
    {
        //打印失败日志
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
            (*it)->Timer_TurnsDecline();
            ++it;
            continue;
        }
        if(p->Timer_GetTurns()==0)
        {
            (*it)->ExecuteCallbackFunc();
            slot[CurrentPos].erase(it);
        }
    }

    CurrentPos=(CurrentPos+1)%SizeOfTW;
}












