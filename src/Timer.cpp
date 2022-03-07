// Created by llf on 2022/3/7.

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


Timer* TimeWheel::TimeWheel_insert_Timer(std::chrono::seconds timeout)
{
    if(timeout<std::chrono::seconds(0)) return nullptr;

    size_t pos=0,cycle=0;
    if(timeout<Si)//向上整合
    {
        cycle=0;
        pos=1;
    }
    else
    {
        cycle=(timeout/Si)/SizeOfTW;
        pos=(CurrentPos+(timeout/Si)%SizeOfTW)%SizeOfTW;
    }
    Timer* Temp=new Timer(pos,cycle);
    slot[pos].push_back(Temp);
    return Temp;
}

bool TimeWheel::TimerWheel_Remove_Timer(Timer* timer)
{
    if(!timer) return false;

    for(auto& it:slot[timer->PosInWheel])
    {
        if(it==timer) {
            delete it;
            return true;
        }
    }
    return false;
}








