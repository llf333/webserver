// Created by llf on 2022/3/8.
//From: https://github.com/progschj/ThreadPool
//解析：https://www.cnblogs.com/chenleideblog/p/12915534.html

#include"ThreadPool.h"

#include<iostream>

Thread_Pool::Thread_Pool(size_t size) :stop_(false),sizeofpoll(size)
{
    for(size_t i=0;i<size;++i)
        //括号里面的lambda表达式作为参数构造thread(thread的构造函数是可调用对象)（emplace_back与push_back的用法）。
    pool.emplace_back([this]() {
        while(true)//每个线程的工作就是无限取任务，执行任务
        {
            //取任务
            std::function<void()> tsk;
            {
                std::unique_lock<std::mutex> lock(this->mtx);

                if(this->stop_&&this->task_que.empty()) return ;//如果线程池停止，且任务队列为空，则终止线程

                while(this->task_que.empty())//当任务队列为空，则一直阻塞,注意使用while，防止虚假唤醒
                    cv.wait(lock);

                //这句主要用于析构函数唤醒后退出
                if(this->stop_&&this->task_que.empty()) return ;//如果线程池停止，且任务队列为空，则终止线程

                tsk=std::move(this->task_que.front());//move更快
                this->task_que.pop();
            }

            //执行任务
            tsk();
        }
    });
}

Thread_Pool::~Thread_Pool()
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop_=true;
    }

    cv.notify_all();

    //等到所有线程退出后，主线程再退出
    for(auto& thread_: pool)
        thread_.join();

    //没有new什么东西，所以这里面没有delete
}



