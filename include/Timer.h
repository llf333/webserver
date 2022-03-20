// Created by llf on 2022/3/7
#ifndef WEBSERVER_TIMER__H
#define WEBSERVER_TIMER__H

#include<functional>
#include<list>
#include<vector>
#include "Other.h"
#include "HttpData.h"

class Chanel;
class HttpData;
/*!
 * @brief 定时器
 * @FuncofTimeUp 超时回调函数
 * @Turns 剩下多少圈
 * @PosInWheel 位于时间轮的哪个槽
 */
class Timer
{
private:
    using CALLBACK=std::function<void()>;
    CALLBACK FuncOfTimeUp;//超时回调函数
    size_t PosInWheel;//位于时间轮的哪个槽
    size_t Turns;//剩下多少圈
    //std::seconds Time_t;不用记录初始时间，在构造时直接传时间通过计算来构造,然后通过上面两个参数来记录实时时间

public:
    size_t Timer_GetPos()   {return PosInWheel;}
    size_t Timer_GetTurns() {return Turns;}
    void Timer_TurnsDecline() {Turns--;}
    Timer(size_t pos,size_t turns);

    friend class TimeWheel;

private:
    void ExecuteCallbackFunc();//之所以是私有的，因为只能通过友元时间轮来调用
};

/*!
 * @brief 时间轮
 * @SizeOfTW 一共几个槽
 * @CurrentPos 当前在哪个槽
 * @Si 走过一个槽代表的时间
 */
class TimeWheel
{
private:
    std::vector< std::list<Timer*> > slot;//时间轮的槽，每个槽中有一个链表
    size_t SizeOfTW;
    size_t CurrentPos=0;//初始在第0个槽
    std::chrono::seconds Si;//一个槽代表经过一秒

private:
    void tick();//tick的功能就是到时后，执行当前槽中的已经到时的定时器，并移动至下一个槽

public:
    int tick_d[2]{};// 用于tick时间轮的管道

public:
    TimeWheel(size_t maxsize);
    TimeWheel(const TimeWheel& wheel) =delete;
    TimeWheel& operator=(const TimeWheel& wheel) =delete;
    ~TimeWheel();
    Timer* TimeWheel_insert_Timer(std::chrono::seconds timeout,HttpData* holder);
    bool TimerWheel_Remove_Timer(Timer* timer);
    bool TimerWheel_Adjust_Timer(Timer* timer,std::chrono::seconds timeout);

};
#endif