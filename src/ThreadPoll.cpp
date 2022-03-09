// Created by llf on 2022/3/8.

#include"ThreadPool.h"

#include<iostream>

Thread_Pool::Thread_Pool(size_t size) :stop_(false)
{
    for(size_t i=0;i<size;++i)
        //括号里面的lambda表达式作为参数构造thread(thread的构造函数是可调用对象)（emplace_back与push_back的用法）。
    pool.emplace_back([this]() {
        while(true)
        {
            std::function<void()> tsk;
            {
                std::unique_lock<std::mutex> lock(this->mtx);

                if(this->stop_&&this->task_que.empty()) return ;//如果线程池停止，且任务队列为空，则终止线程

                while(this->task_que.empty())//当线程池停止了或者任务队列为空，则一直阻塞
                    cv.wait(lock);

                //析构函数唤醒后退出
                if(this->stop_&&this->task_que.empty()) return ;//如果线程池停止，且任务队列为空，则终止线程

                tsk=std::move(this->task_que.front());
                this->task_que.pop();
            }
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



