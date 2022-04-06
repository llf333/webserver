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

    /*创建一个管道，通过监听tick_fd_[0]可读事件的形式定时让时间轮tick一下*/
    //llf 往tick_fd_[1]中写数据可以通知时间轮tick一下
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
    /*delete所有timer*/
    for(auto& l:slot)
        for(auto& timer:l)
            delete timer;

    //关闭管道
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
    slot[pos].push_back(Temp);//插入到对应的槽当中

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

    //有错，槽的位置可能发生改变
    int old_pos=timer->Timer_GetPos();
    slot[old_pos].remove(timer);//移除但不要delete
    timer->Timer_SetPos(pos);
    timer->Timer_SetTurns(cycle);
    slot[pos].push_back(timer);//重新插入到对应位置

    return true;
}

//2022/03/12
void TimeWheel::tick()
{
    //先把管道里的数据读了，避免堵住了
     char buffer[128];
    int ret=Read_from_fd(tick_d[0],buffer, strlen("tick\0"));

    if(ret<=0)
    {
        //打印失败日志
        Getlogger()->error("in tick ,read data to buffer error  {} ，ret={}", strerror(errno),ret);
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

        /*圈数大于0说明还未到触发时间*/
        if(p->Timer_GetTurns()>0)
        {
            p->Timer_TurnsDecline();
            ++it;
            continue;
        }

        //否则说明到触发时间了
        else
        {
            p->ExecuteCallbackFunc();
            slot[CurrentPos].erase(it);
        }
    }

    CurrentPos=(CurrentPos+1)%SizeOfTW; //指向下一个槽
}












