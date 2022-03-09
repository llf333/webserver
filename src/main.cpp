#include<iostream>
#include"Timer.h"
#include"ThreadPool.h"

int main() {
    Thread_Pool a(10);
    std::vector<std::future<int>> res;
    for(int i=0;i<10;i++)
    res.emplace_back(a.Add_task([i](){
        std::cout<<"thread "<<i<<std::endl;
        return i*i;
    }));

     for(int i=0;i<10;++i)
      std::cout<<res[i].get()<<std::endl;

    return 0;
}
