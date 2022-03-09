// Created by llf on 2022/3/8
#ifndef WEBSERVER_THREAD_POOL_H
#define WEBSERVER_THREAD_POOL_H

#include<mutex>
#include<condition_variable>
#include<functional>
#include<queue>
#include<vector>

#include<future>

class Thread_Pool
{
private:
    typedef std::function<void()> task;
    std::condition_variable cv;
    std::mutex mtx;
    std::queue<task> task_que;
    std::vector<std::thread> pool;
    bool stop_;

public:
    explicit Thread_Pool(size_t size);
    ~Thread_Pool();

    template<class F,class ...Args>
    auto Add_task(F&& f,Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> ;//这里要加typename，因为是嵌套从属类型，具体可以参考Effective C++
};

//模板函数要定义在头文件中,否则会出现连接错误。

//<<C++ Primer>>:
//模板包含两种名字：
//1.那些不依赖于模板参数的名字。
//2.那些依赖于模板参数的名字。

//当使用模板时，所有不依赖于模板参数的名字都必须是可见的，这是由模板的提供者来保证的。
// **而且，模板的提供者必须保证，当模板被实例化时，模板的定义，包括类模板的成员的定义，也必须是可见的。
template<class F,class ...Args>
auto Thread_Pool::Add_task(F&& f,Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    typedef typename std::result_of<F(Args...)>::type ReturnType;//注意加typename ，嵌套从属类型

    auto tsk=std::make_shared<std::packaged_task<ReturnType()>>
            (std::bind(std::forward<F>(f),std::forward<Args>(args)...));//tsk是个函数指针

    std::future<ReturnType> res=tsk->get_future();

    {
        std::unique_lock<std::mutex> lock(mtx);

        if(stop_)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        task_que.emplace([tsk](){*tsk;});//???????可以直接解引用吗
    }

    cv.notify_one();
    return res;//返回一个任务的future
}

#endif