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
    sigset_t* sigset=reinterpret_cast<sigset_t*>(set);//信号集，表示一组信号
    bool stop=false;
    while(!stop)
    {
        int ret,sig=0;

        //llf 监听信号集sigset中所包含的信号，并将其存在sig中。注意：sigwait函数所监听的信号在之前必须被阻塞。
        //sigwait函数将阻塞调用他的线程，直到收到它所监听的信号发生了，然后sigwait将其从未决队列中取出（因为被阻塞了，所以肯定是未决了）
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
        else if(sig == SIGALRM) //llf 时间轮走过一个槽
        {
            std::string msg="its time to tick";

            //llf 定时器每次到时就tick一下每个subreactor
            //也就是说，每经过一个alarm周期，就对所有的subreactor所对应的时间轮里面每一个定时器进行检验，若时间到了，就执行回调函数
            //只有连接socket才有定时器，用以通知连接超时
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

int main(int argc,char* argv[])//llf argv[0]表示程序的名称
{
    /*解析命令行参数*/
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

    /*!ddj
       在Linux当中，进程的所有线程共享信号。线程可以通过设置信号掩码来屏蔽掉某些信号。
       然而，在这里子线程是通过线程池创建的，不太好添加信号掩码，况且在每个线程中单独
       设置信号掩码也很容易导致逻辑错误。因此，最好的方法是专门定义一个线程去处理所有
       信号。首先需要在所有子线程创建之前，在主线程中设置好信号掩码。随后创建的子线程
       会自动继承这个信号掩码。这样做了之后，所有线程都不会响应被屏蔽的信号。因此，需
       要再单独创建一个线程，并通过调用sigwait函数来等待信号并处理。
     */
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
    /*开启一个后台信号处理函数*/
    std::thread Sig_thread(sig_thread,&set);
    Sig_thread.detach();

    /*创建一个线程池。注意主线程不在线程池中*/
    Thread_Pool* main_thread_pool=new Thread_Pool(std::get<1>(*res));
    EventLoop* main_reactor=new EventLoop(true);

    alarm(GlobalValue::TimeWheel_PerSlotTime);//这个时间应该是时间轮每一槽经过的时间

    //单例模式
    main_server=SERVER::Get_the_service(std::get<0>(*res),main_reactor,main_thread_pool);

    /*服务器开始运行*/
    main_server->Server_Start();
    main_reactor->StartLoop();
    return 0;
}
