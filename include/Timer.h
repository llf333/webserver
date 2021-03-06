// Created by llf on 2022/3/7
#ifndef WEBSERVER_TIMER__H
#define WEBSERVER_TIMER__H

#include<functional>
#include<list>
#include<vector>
#include <chrono>


class Chanel;
class HttpData;


/*!
 * @brief 定时器
 * @FuncofTimeUp 超时回调函数
 * @Turns 剩下多少圈
 * @PosInWheel 位于时间轮的哪个槽
 */
class Timer //timer指针在bool TimeWheel::TimerWheel_Remove_Timer(Timer* timer)中删除
            //在insert中新建
{
private:
    using CALLBACK=std::function<void()>;
    CALLBACK FuncOfTimeUp;//超时回调函数
    size_t PosInWheel;//位于时间轮的哪个槽
    size_t Turns;//剩下多少圈
    //std::seconds Time_t;不用记录初始时间，在构造时直接传时间通过计算来构造,然后通过上面两个参数来记录实时时间

public:
    Timer(size_t pos,size_t turns);

    size_t Timer_GetPos()                           {return PosInWheel;}
    void Timer_SetPos(size_t new_pos)               {PosInWheel=new_pos;}

    size_t Timer_GetTurns()                         {return Turns;}
    void Timer_SetTurns(size_t new_turns)           {Turns=new_turns;}

    void Timer_TurnsDecline()                       {Turns--;}

    friend class TimeWheel;

    void Register_CallbackFunc(CALLBACK func)       {FuncOfTimeUp=func;}

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
    size_t SizeOfTW;//一轮几个槽
    size_t CurrentPos=0;//初始在第0个槽
    std::chrono::seconds Si;//一个槽代表经过一秒

    Chanel* tick_chanel;//监听tick[0]的事件-----------什么时候删除
    int tick_d[2]{};// 用于tick时间轮的管道，tick[1]为写，tick[0]表示读

private:
    void tick();//tick的功能就是到时后，执行当前槽中的已经到时的定时器，并移动至下一个槽

public:
    TimeWheel(size_t maxsize=12);
    TimeWheel(const TimeWheel& wheel) =delete;
    TimeWheel& operator=(const TimeWheel& wheel) =delete;
    ~TimeWheel();

    //Timer* TimeWheel_insert_Timer(std::chrono::seconds timeout,HttpData* holder);//应该是通用的事件器，这种定义不好

    Timer* TimeWheel_insert_Timer(std::chrono::seconds timeout);
    bool TimerWheel_Remove_Timer(Timer* timer);
    bool TimerWheel_Adjust_Timer(Timer* timer,std::chrono::seconds timeout);

    Chanel* Get_tickChanel(){return tick_chanel;};
    int Get_1tick(){return tick_d[1];};

};

#endif