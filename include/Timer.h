#ifndef WEBSERVER_TIMER__H
#define WEBSERVER_TIMER__H

#include<functional>
#include<list>
#include<vector>
#include<chrono>

class Chanel;

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
    Timer(size_t pos,size_t turns);

    friend class TimeWheel;

private:
    void ExecuteCallbackFunc();
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

public:
    TimeWheel(size_t maxsize);
    ~TimeWheel();
    Timer* TimeWheel_insert_Timer(std::chrono::seconds timeout);

    bool TimerWheel_Remove_Timer(Timer* timer);
    void tick();//tick的功能就是到时后，执行当前槽中的已经到时的定时器，并移动至下一个槽
};

#endif