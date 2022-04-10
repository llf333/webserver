# WebServer学习笔记



## C++ 11之后的新功能



### Using typename

```c++
using return_type=typename std::result_of<F(Args...)>:: type;
//这里的typename是为了说明后面是个类型,因为后面是个嵌套从属类型
```



### std::result_of
​		用于在编译的时候推导出一个可调用对象（函数,std::funciton或者重载了operator()操作的对象等）的返回值类型.主要用于模板编写中.



### std::optional——C++17
​		std::optional<具体类型> 是一个包含0或1个元素的容器。作为返回值可以是返回**std::nullopt**，也可以是返回**具体类型**。



### std::tuple

​		std::tuple是类似pair的模板。每个pair的成员类型都不相同，但每个pair都恰好有两个成员。不同std::tuple类型的成员类型也不相同，但**一个std::tuple可以有任意数量的成员**。每个确定的std::tuple类型的成员数目是固定的，但一个std::tuple类型的成员数目可以与另一个std::tuple类型不同。
但我们希望将一些数据组合成单一对象，但又不想麻烦地定义一个新数据结构来表示这些数据时，std::tuple是非常有用的。我们可以将std::tuple看作一个”快速而随意”的数据结构。



### std::bind

```c++
auto newCallable = bind(callable,arg_list);
```

​		arg_list中的参数可能包含形如n的名字，其中n是一个整数，这些参数是“占位符”，表示**newCallable的参数**，它们占据了传递给newCallable的参数的“位置”。数值n表示生成的可调用对象中参数的位置：\_1为newCallable的第一个参数，_2为第二个参数，以此类推。

​		在**WEBSERVER**项目写线程池时，主要目的就是**把很多类型不同的函数都包装成 void name () 类型，用于模板编程，提高复用性。**



### std::future

​		我们想要从线程中返回异步任务结果，一般需要依靠**全局变量**；从安全角度看，有些不妥；为此C++11提供了std::future类模板，**future对象提供访问异步操作结果的机制**，很轻松解决从异步任务中返回结果。



### std::packaged_task

​		简单来说std::packaged_task<F>是对可调对象(如函数、lambda表达式等)进行了包装，并将这一可调对象的返回结果传递给关联的std::future对象。

```c++
#include<functional>
#include<future>
#include<thread>
#include<iostream>

int test(int a)
{
    a+=10;
    return a;
}

int main()
{
    std::packaged_task<int(int)> fun1(test);
    std::future<int> res=fun1.get_future();
    
    std::thread t1(fun1,100);//切记：传入线程中的是pack打包好的函数。而不是上面那个。
    
    std::cout<<res.get()<<std::endl;
    return 0;
    
}
```



### std::forward

​		待看————Effective Modern C++ 关于完美转发的内容



### std::chrono

​		代表时间。常用手段：**std::chrono::seconds(5);**



   ### atomic原子量

​		在头文件\<atomic>，保证对变量的操作都是原子操作。

​		使用方法：

```c++
#include<atomic>
std::atomic<int> a=0;
```



### mutex互斥量

​		在头文件\<mutex>中。

​		使用方法

```c++
#include<mutex>

std::mutex mtx1;
mtx1.lock();
a++;//若在临界区return或者抛出异常，则有可能死锁
mtx1.unlock();

//解决方法，使用std::unique_lock，若在临界区return或者抛出异常，则会自动析构

//注意并不是unique_mutex.
std::unique_lock<std::mutex> lock(mtx1);//构造函数需要传一个mutex，这一步实际上已经上锁
lock.unlock();//解锁

```



### 条件变量

​		在头文件\<condition_variable>中

​		使用方法：

```c++
#include<thread>
#include<iostream>
#include<chrono>
#include<queue>
#include<mutex>
#include<condition_variable>

std::queue<int> q;//工作队列
std::mutex mtx1;
int i=0;
std::condition_variable cv;

void consumer()
{
    while(1)
    {
        if(q.size())
        {
            std::unique_lock<std::mutex> lock(mtx1);

            while(q.empty())//要使用while，否则可能虚假唤醒
            {
                cv.wait(lock);//wait时会释放锁
            }
            int t=q.front();
            q.pop();
            std::cout<<t<<std::endl;
        }
    }
}

void producer()
{
    while(1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(mtx1);
        q.push(i++);
        cv.notify_one();//唤醒一个线程
    }
}

int main()
{
    std::thread Cosu(consumer);
    std::thread Produ(producer);
    Cosu.join();
    Produ.join();
    return 0;
}

```



### 虚假唤醒

​		假设是在等待消费队列，一个线程A被nodify，但是还没有获得锁时，另一个线程B获得了锁，并消费掉了队列中的数据。B退出或wait后，A获得了锁，而这时条件已不满足。



### 信号量

```c++
#include<iostream>
#include<semaphore>
#include<thread>

std::counting_semaphore<4> csem(0);
std::binary_semaphore bsem(0);

void func()
{
    std::cout<<"thread want to get the semaphore !"<<std::endl;
    csem.acquire();
    std::cout<<"thread get !"<<std::endl;
}

int main()
{
    std::thread task1(func);
    std::thread task2(func);
    std::thread task3(func);
    std::thread task4(func);
    
    std::cout<<"semaphore is ready to release 2 !"<<std::endl;
    csem.release(2);
    std::cout<<"semaphore has been released !"<<std::endl;
    
    task1.join();
    task2.join();
    task3.join();
    task4.join();
}
```



### std::call_once

​		某些场景下，我们需要代码只被执行一次，比如单例类的初始化，考虑到多线程安全，需要进行加锁控制。C++11中提供的call_once可以很好的满足这种需求，使用又非常简单。

```c++
#include<mutex>

template <class Fn, class... Args>

void call_once (std::once_flag& flag, Fn&& fn, Args&&...args);
```

​		第一个参数是std::once_flag的对象(once_flag是不允许修改的，其拷贝构造函数和operator=函数都声明为delete)，第二个参数可调用实体，即要求只执行一次的代码，后面可变参数是其参数列表。





## 一些知识点

### g++支持C++20

​		在编译时，末尾加上-std=c++20



### emplace_back和push_back的区别

​		push_back：在引入右值引用，转移构造函数，转移复制运算符之前，通常使用push_back()向容器中加入一个右值元素（临时对象）的时候，首先会调用构造函数**构造这个临时对象**，然后需要调用拷贝构造函数将这个临时对象放入容器中。原来的**临时变量释放**。这样造成的问题是临时变量申请的**资源就浪费**。push_back括号里面是对象，以该对象拷贝构造。

​		emplace_back：在容器尾部添加一个元素，这个元素**原地构造**，不需要触发拷贝构造和转移构造。而且调用形式更加简洁，直接根据参数初始化临时对象的成员。emplace_back括号里面是参数，以该参数原地构造。



### 信号处理方式——同步处理与异步处理

​		在Linux的**多线程中使用信号机制**，与在进程中使用信号机制有着根本的区别，可以说是完全不同。在进程环境中，对信号的处理是，先注册信号处理函数，当信号异步发生时，调用处理函数来处理信号。它完全是异步的（我们完全不知到信号会在进程的那个执行点到来！）。然而信号处理函数的实现，有着许多的限制；比如有一些函数不能在信号处理函数中调用；再比如一些函数read、recv等调用时会被异步的信号给中断(interrupt)，因此我们必须对在这些函数在调用时因为信号而中断的情况进行处理（判断函数返回时 enno 是否等于 EINTR）。

​		但是在多线程中处理信号的原则却完全不同，它的基本原则是：将对信号的异步处理，转换成同步处理，也就是说**用一个线程专门的来“同步等待”信号的到来，而其它的线程可以完全不被该信号中断/打断(interrupt)**。这样就在相当程度上简化了在多线程环境中对信号的处理。而且可以保证其它的线程不受信号的影响。这样我们对信号就可以完全预测，因为它不再是异步的，而是同步的（我们完全知道信号会在哪个线程中的哪个执行点到来而被处理！）。而同步的编程模式总是比异步的编程模式简单。其实多线程相比于多进程的其中一个优点就是：多线程可以将进程中异步的东西转换成同步的来处理。

​		sigwait是**同步**的等待信号的到来，而不是像进程中那样是异步的等待信号的到来。sigwait函数使用一个信号集作为他的参数，并且在集合中的任一个信号发生时返回该信号值，**解除阻塞**，然后可以针对该信号进行一些相应的处理。

​		调用sigwait同步等待的信号必须在调用线程中被屏蔽，并且通常应该在所有的线程中被屏蔽（这样可以保证信号绝不会被送到除了调用sigwait的任何其它线程），这是通过利用信号掩码的继承关系来达到的。

​		Webserver中，在每个时间轮中，都有一对管道，时间轮的定时tick功能是通过alarm函数定时产生一个SIGALRM信号，并在该信号的处理函数中向一个管道的一端写数据，使得管道的另一端触发EPOLLIN事件，从而在epoll_wait返回后调用相应的回调函数使得时间轮tick一下。然而，epoll_wait函数的阻塞调用会被系统信号中断，若SubReactor没有屏蔽SIGALRM信号，就会导致epoll_wait会被定时打断，影响程序的稳定性。

​		在Linux当中，进程的所有线程共享信号。线程可以通过设置信号掩码来屏蔽掉某些信号。然而，在这里子线程是通过线程池创建的，不太好添加信号掩码，况且在每个线程中单独设置信号掩码也很容易导致逻辑错误。因此，最好的方法是专门定义一个线程去处理所有信号。首先需要在所有子线程创建之前，在主线程中设置好信号掩码。随后创建的子线程会自动继承这个信号掩码。这样做了之后，所有线程都不会响应被屏蔽的信号。因此，需要再单独创建一个线程，并通过调用sigwait函数来等待信号并处理。

​		SIGTERM是结束进程，调用终止函数。

​		SIGALRM则向时间轮管道写数据，通知tick。

​		SIGPIPE是直接do nothing，如果不处理这个信号，默认行为是关闭进程。

​        ① 初始时，C、S连接建立，若某一时刻，C端进程关机或者被KILL而终止（终止的C端进程将会关闭打开的文件描述符，即向S端发送FIN段），S端收到FIN后，响应ACK
​        ② 假设此时，S端仍然向C端发送数据：当第一次写数据后，S端将会收到RST分节； 当收到RST分节后，第二次写数据后，S端将收到SIGPIPE信号（S端进程被终止）            



### 为什么监听socket一定要是非阻塞的

​	（前提是采用了IO复用函数）

​		当一个连接到来的时候，监听套接字可读，此时，我们稍微等一段时间之后再调用accept()。就在这段时间内，客户端**设置linger选项(l_onoff = 1, l_linger = 0)**，然后**调用了close()**，那么客户端将不经过四次挥手过程，通过发送**RST报文**断开连接。服务端接收到RST报文，系统会将排队的这个未完成连接直接删除，此时就相当于没有任何的连接请求到来， 而**接着调用的accept()将会被阻塞**（阻塞整个线程），直到另外的新连接到来时才会返回。这是与IO多路复用的思想相违背的**(系统不阻塞在某个具体的IO操作上，而是阻塞在select、poll、epoll这些IO复用上的)**。

​		上述这种情况下，**如果监听套接字为非阻塞的，accept()不会阻塞住，立即返回-1，同时errno = EWOULDBLOCK。**



### Epoll在使用ET模式时，为什么要把监听的套接字设为非阻塞模式？

​		如果多个连接同时到达，ET模式下就只会通知一次，为了处理剩余的连接数，必须要时刻accpet 句柄，直到出现errno为EAGAIN, 出于这样的目的的话，socket也要设置为非阻塞



### socket客户端连接上服务端是在listen之后而非在accept之时

​		在listen后，客户端就可以与服务器进行连接（TCP三次握手），此时的连接结果放在队列中。服务端知乎调用 accept() 函数从队列中获取一个已准备好的连接，函数返回一个新的 socket ,新的 socket 用于与客户端通信，listen 的 socket 只负责监听客户端的连接请求。



### 事件处理模式

​		开发平台以及部署平台都是Linux，因而选择Reactor模式。这是因为Linux下的异步I/O是不完善的，aio系列函数是在用户空间出来的，不是由操作系统支持的，故部署在Linux上的高性能服务器大多选择Reactor模式。Reactor模式有三种常用方案：

- 单Reactor单进程/线程
- 单Reactor多线程
- 多Reactor多线程

​		单Reactor单进程/线程方案实现起来非常简单，不涉及任何进程或线程间的通信，但缺点也是十分明显的。一旦业务逻辑处理速度过慢，将导致严重的延迟响应。因此，单Reactor单进程/线程模式适合业务处理较为快速的场景。单Reactor多线程解决了上一个方案的问题，但由于只有一个Reactor负责所有事件的监听和响应，无法有效应对瞬时高并发的情形；多Reactor多线程不存在以上两个方案的问题，且线程间的通信也很简单，MainReactor只需将连接socket传递给SubReactor即可。数据的收发以及业务逻辑均在SubReactor中完成，任务划分十分明确。因此，本项目选择采用多Reactor多线程方案。



### 类说明

- Channel: 底层事件类。可视作一个文件描述符、其需要监听的事件以及这些事件相应回调函数的封装。
- HttpData: http数据处理类。包括了连接socket所有事件的回调函数。
- EventLoop: 即reactor，负责向epoll_wait中添加、修改、删除事件以及调用epoll_wait并根据活跃事件调用其相应的回调函数。



### 连接的建立与分发

​		这部分的功能由主线程，也即MainReactor完成。在本项目中，reactor的角色由类Eventloop扮演。在主线程中的Eventloop对象为MainReactor，子线程中的Eventloop对象即为SubReactor，并且采用了one loop per thread的模式。在MainReactor中只需要监听一个listen socket的可读（EPOLLIN）以及异常事件（EPOLLERR）。在可读事件的回调函数中，包括了ET模式下的accept函数调用以及连接socket的分发。其中连接socket会被分发给当前监听事件最少的SubReactor。连接socket的EPOLLIN、EPOLLOUT、EPOLLERR以及EPOLLRDHCP均由SubReactor负责监听和相应回调函数的调用。



### 请求报文的解析与响应报文的发送

​		建立了tcp连接后，服务器会针对http请求报文的请求行和首部行数据的完整到达设置一个超时时间，若在该时间内，服务端没有接受到完整的http请求报文，那么会向客户端发送408 request time-out并关闭连接。同时，对于post报文中的实体数据，要求相邻两实体数据包到达的时间间隔不超过一个特定的时间，否则发送408并关闭连接。



### 如何关闭连接

​		服务端关闭连接分为主动关闭和被动关闭两种情况。在连接socket发生EPOLLERR、从连接socket读取数据时出错、向连接socket写数据出错以及处理完短连接请求时，服务端会主动关闭连接。被动关闭连接发生在客户端首先先主动关闭了连接，此时服务端也对等关闭连接。



### SO_REUSEADDR选项

​		\*SO_REUSEADDR允许启动一个监听服务器并捆绑其众所周知的端口，即使以前建立的将该端口用作他们的本地端口的连接仍存在。

​		SO_REUSEADDR允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑一个不同的本地IP地址即可



### TCP_NODELAY选项

​		TCP/IP协议中针对TCP默认开启了Nagle算法。Nagle算法通过减少需要传输的数据包，来优化网络。在内核实现中，数据包的发送和接受会先做缓存，分别对应于写缓存和读缓存。启动TCP_NODELAY，就意味着禁用了Nagle算法，允许小包的发送。对于延时敏感型，同时数据传输量比较小的应用，开启TCP_NODELAY选项无疑是一个正确的选择。 比如，对于SSH会话，用户在远程敲击键盘发出指令的速度相对于网络带宽能力来说，绝对不是在一个量级上的，所以数据传输非常少； 而又要求用户的输入能够及时获得返回，有较低的延时。如果开启了Nagle算法，就很可能出现频繁的延时，导致用户体验极差。



### epoll中max_event事件的确定

​		超过 maxevents 的就绪事件会被抛弃吗？-----不会，超过的仍然挂在epoll的就绪任务队列中，没有取出来。通常maxevents 必须要小于epoll_wait第二个参数数组的大小，其实它的主要目的是防止越界访问event数组。换句话说，max_event主要影响的是并行程度，即一次调用能处理多少事件，特别小会限制并行度，特别大通常没有什么问题。



### 报文分批到达的处理

​		有没有可能客户端发送的http请求报文由于网络受限从而分多次到达，进而可能导致一次ReadData函数调用无法获取完整的请求报文，从而让服务器误以为请求报文的语法不正确而关闭连接？-----不会，读到不完整数据直接返回，等待下次EPOLLIN事件。



### 写数据时缓冲区满的处理

​		write函数和send函数可能因为客户端或者服务端的缓冲已满，而不能继续写数据，从而可能导致一次WriteData函数不能把响应报文写完，这种情况又该如何处理？-----首先删除定时器，避免因超时而没写完数据。然后重新注册EPOLLOUT事件，在下轮epoll监听时重新试着写数据，



### 时间器时间更改的几个节点

​		1、建立连接时，时间器定时为header_time，Epoll

​		2、分析到post请求时，判断数据是否完整前，时间器更改为body_time

​		3、完成发送后，重置时间：如果为长连接，重置时间为keep_alive_time。如果是短连接，则删除is_conn事件，并删除时间器。



### backlog参数

​		从内核2.2版本之后，listen函数的backlog参数表示的是全连接的数量上限。所谓全连接，指的是完成了tcp三次握手处于establish状态的连接。在listen后，客户端就可以与服务器进行连接（TCP三次握手），此时的连接结果放在队列中。服务端调用 accept() 函数从队列中获取一个已准备好的连接，函数返回一个新的 socket ,新的 socket 用于与客户端通信。backlog指的就是accept队列的长度在任何时候都最多只能为backlog。在5.4版本之后backlog的默认最大值为4096(定义在/proc/sys/net/core/somaxconn)。显然，backlog与服务器的最大并发连接数量没有直接的关系（accept得够快（从队列中取得够快）），只会影响服务器允许同时发起连接的客户端的数量。

[Linux中，Tomcat 怎么承载高并发（深入Tcp参数 backlog） - 三国梦回 - 博客园 (cnblogs.com)](https://www.cnblogs.com/grey-wolf/p/10999342.html)

​		全连接队列满了怎么办？ -----在 accept 队列满的情况下，收到了三次握手中的最后一次 ack 包， 它就直接无视这个包。（此时，客户端是全连接状态（收到了第二次握手的报文），服务端是半连接状态（无视了第三次握手的报文）） 一开始，看起来有点奇怪，但是记得， 服务端SYN RECEIVED 状态（半连接状态）下的 socket 有一个定时器。

​		该定时器的机制： 如果 ack 包没收到（或者被无视，就像我们上面描述的这个情况）， tcp 协议栈 会重发 SYN/ACK （第二次握手）包。（重发次数由 /proc/sys/net/ipv4/tcp_synack_retries 指定）。







## 值得记录的BUG

### 1

C++的类体中，方法以外的区域不允许有初始化，简单类型是可以的，但是有构造函数的复杂对象则不行了，比如string对象！

```cpp
class A
{    
     vector<string> v(9);  //error,expected identifier before numeric constant
public:
     void test(){}
};
```



### 2

定义全局变量时使用static，意味着该变量的作用域只限于定义它的**源文件**中，其它源文件不能访问。如果这种定义方式出现在头文件中，那么可以很自然地推测：**包含了该头文件的所有源文件中都定义了这些变量，即该头文件被包含了多少次，这些变量就定义了多少次**。



### 3

string += 时，后面不能一直加 “ ”形式的字符串，从第二个开始要显式地定义类型。



### 4

模板函数要定义在头文件中,否则会出现连接错误。



### 5

4/8严重bug（4天），现象：无法正常断开连接，断开连接时出现乱七八糟的fd，以及读数据出错。

​		起初我以为是内存泄漏问题，重新加以整理并修改（很大改善，log中error明显减少，但仍有问题）

​		然后我以为状态机解析出错，导致没有断开连接一直触发EPOLLIN，后面打断点发现并不是。

​		然后对调用回调函数的过程进行了修改，修改成一次只调用一个事件（很大改善），一次只调用一次之后就考虑到事件优先级（在反复尝试之后并无改善）

​		最后好奇断开连接时触发了什么事件，查资料解释如下：

​		**客户端断开链接，服务端这边会触发EPOLLIN，EPOLLOUT，EPOLLRDHUP事件，有些人可能会在服务端关心EPOLLRDHUP事件，触发后关闭套接字，但是这个处理逻辑不是通用的，有些系统（老的linux系统）未必会触发EPOLLRDHUP。**
​		最常用的做法是关心EPOLLIN事件，然后在read的时候进行处理：

1. **read返回0，对方正常调用close关闭链接**

2. **read返回-1，需要通过errno来判断，如果不是EAGAIN和EINTR，那么就是对方异常断开链接两种情况服务端都要close套接字**

   

   之前的写法，是读错误了仍然返回已读数据量(return writes_sum)，但是这样就无法在外面调用断开连接，最后将这里改为return -1;





## Linux高性能书的一些记录



### 统一事件源

​		统一事件源的逻辑：把信号包装成一个事件，添加进多路复用函数事件集里进行统一处理。

​		**原本的信号处理函数的职能减小，只将信号传入管道。主函数通过监听管道来真正地完成信号处理。**





### 非阻塞读和阻塞写

​		阻塞IO: socket 的阻塞模式意味着必须要**做完**IO 操作（包括错误）**才会返回**。
​		非阻塞IO: 非阻塞模式下无论操作是否完成都会**立刻返回**，**需要通过其他方式**来判断具体操作是否成功。(对于connect，accpet操作，通过select判断，对于recv，recvfrom，send，sendto通过返回值+错误码来判断)

​		***为什么读操作一般是非阻塞的，而写是阻塞的？***-----读，究其原因主要是读数据的时候我们并**不知道对端到底有没有数据**，数据是在什么时候结束发送的，**如果一直等待就可能会造成死循环**，所以并没有去进行这方面的处理；写，而对于write, **由于需要写的长度是已知的，所以可以一直再写，直到写完．**不过问题是write 是可能被打断吗，造成write 一次只write 一部分数据, 所以write 的过程还是需要**考虑循环write,** 只不过多数情况下一次write 调用就可能成功.





### gdb调试printf记得打空行，否则看不到输出信息

