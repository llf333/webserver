//
// Created by llf on 2022/3/9.
//
#include<iostream>
#include"Timer.h"
#include"ThreadPool.h"

int main() {
    Thread_Pool a(10);//构造时，子线程就在不断地取任务和完成任务。
    std::vector<std::future<int>> res;//注意结果的类型。
    for(int i=0;i<10;i++)
        res.emplace_back(a.Add_task([i](){
            std::cout<<"thread "<<i<<std::endl;
            return i*i;
        }));

    for(int i=0;i<10;++i)
        std::cout<<res[i].get()<<std::endl;//若没完成任务，会一直阻塞

    return 0;
}


