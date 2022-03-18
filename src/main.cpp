#include<iostream>
#include<utility>

#include"Timer.h"
#include"ThreadPool.h"
#include"Chanel.h"
#include"EventLoop.h"
#include"Other.h"
#include"Server.h"

#include<signal.h>


static void sig_thread(void* set)
{
    sigset_t* sigset=reinterpret_cast<sigset_t*>(set);
    bool stop=false;
    while(!stop)
    {
        int ret,sig=0;
        ret= sigwait(sigset,&sig);

        if(ret!=0)
        {
            Getlogger()->error("sigwait error: {}", strerror(ret));
            exit(1) ;
        }

        else if(sig == SIGTERM)
        {
            stop=true;
            //delete所有资源并退出程序
        }
        else if(sig == SIGALRM)
        {

        }
        else if(sig == SIGPIPE)
        {

        }
    }
}

int main(int argc,char* argv[])
{
    auto res= AnalyseCommandLine(argc,argv);
    if(res==std::nullopt)
    {
        std::cout<<"The command line is wrong! "<<std::endl;
        std::cout<<"Please use the form :: ./server -p Port -s SubReactorSize -l LogPath(start with ./)"<<std::endl;
    }

    //开启日志
    if(!Getlogger(std::get<2>(*res)))
    {
        std::cout<<"Failed to create a Logger ! "<<std::endl;
        return 0;
    }
    Getlogger()->set_pattern("[%Y-%m-%d %H:%M:%S] [thread %t] [%l] %v");

    //先屏蔽某些信号，再单开一个线程进行处理。保证线程池中的线程不会被这些信号干扰（屏蔽后，子线程也随之屏蔽）
    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set, SIGALRM);
    sigaddset(&set,SIGTERM);
    sigaddset(&set,SIGPIPE);

    if(pthread_sigmask(SIG_SETMASK,&set,NULL)!=0)
    {
        std::cout<<"Failed to mask the signal in the main function ! "<<std::endl;
        return 0;
    }
    std::thread Sig_thread(sig_thread,&set);
    Sig_thread.detach();

    Thread_Pool* main_thread_pool=new Thread_Pool(std::get<1>(*res));
    EventLoop* main_reactor=new EventLoop(true);

    alarm(10);

    SERVER* main_server=SERVER::Get_the_service(std::get<0>(*res),main_reactor,main_thread_pool);

    main_server->Server_Start();

    main_reactor->StartLoop();

    return 0;
}
