#include<iostream>
#include<utility>

#include"Timer.h"
#include"ThreadPool.h"
#include"Chanel.h"
#include"EventLoop.h"
#include"Other.h"
#include"Server.h"
#include"HttpData.h"

#include<signal.h>

static SERVER* main_server;//为什么要写成全局，一是它是唯一的，二是方便信号处理函数调用

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
            main_server->Server_Stop();
            delete main_server;
            exit(0);
        }
        else if(sig == SIGALRM)
        {
            std::string msg="its time to tick";
            for(auto& it:main_server->timeWheel_PipeOfWrite)
            {
                const char* msg="tick\0";//随便写点消息
                int ret= Write_to_fd(it,msg, strlen(msg));
                if(ret==-1)
                {
                    Getlogger()->error("tick error in sig_thread");
                    exit(-1);
                }
                alarm(GlobalValue::TimeWheel_PerSlotTime);
            }
        }
        else if(sig == SIGPIPE)
        {
            // do nothing

            //如果不处理这个信号，默认行为是关闭进程

            //SIGPIPE什么时候被收到：
            //① 初始时，C、S连接建立，若某一时刻，C端进程关机或者被KILL而终止（终止的C端进程将会关闭打开的文件描述符，即向S端发送FIN段），S端收到FIN后，响应ACK
            //② 假设此时，S端仍然向C端发送数据：当第一次写数据后，S端将会收到RST分节； 当收到RST分节后，第二次写数据后，S端将收到SIGPIPE信号（S端进程被终止）

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
        return 0;
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

    alarm(GlobalValue::TimeWheel_PerSlotTime);//这个时间应该是时间轮每一槽经过的时间

    //单例模式
    main_server=SERVER::Get_the_service(std::get<0>(*res),main_reactor,main_thread_pool);

    main_server->Server_Start();

    main_reactor->StartLoop();

    return 0;
}
