// Created by llf on 2022/3/11.

#include "Chanel.h"

Chanel::Chanel(int _fd, bool is_conn_): fd(_fd),is_connect(is_conn_)
{
}

Chanel::~Chanel()
{
    close(fd);
}

void Chanel::CallRevents()
{
    //4/9
    //疑问：ET模式不是只通知一次吗，这里一次只处理一个事件是否合理？
    //答：ET模式是只通知一次某个事件，而不是所有事件。

    if(revents & EPOLLERR)
        CallErfunc();

    else if(revents & EPOLLRDHUP) //4/7日，在修复长连接出错时调换了顺序，原顺序是 err-in-out-rdhub,会导致一直读数据
        //                               修改为  err-out-rdhub-in
        //按优先级排序，rdhub放在后面会导致一直无法正常断开连接，因为一次只处理一个事件----好像是错的 4/9
        CallDiscfunc();

    else if(revents & EPOLLOUT)
        CallWrfunc();

    else if(revents & EPOLLIN)
        //4/5，重大bug————长时间没判断出为什么会有莫名奇妙的fd值（特别大或者是负数），log中总是显示删除fd失败，写数据失败。
        //怀疑是内存泄漏，于是从头到尾地明确了一下各个对象是什么时候被delete的，原版httpdata是用unique_ptr管理的，
        // 但是我觉得在eventloop中存储httpdata的没有必要，因此手动删除了httpdata。

        //上述处理后仍然有问题，在确定已经正确地删除了httpdata之后，仍然有莫名奇妙的fd值，怀疑是删了之后非法调用，最终改了这里得以解决问题，一次监听只处理一个事件。但是理解得还不够透彻
        CallRdfunc();

}

void Chanel::CallRdfunc()
{
    if(read_handle) read_handle();
    else;//打印日志：还没注册
}

void Chanel::CallWrfunc()
{
    if(write_hande) write_hande();
    else ;//打印日志：还没注册
}
void Chanel::CallErfunc()
{
    if(error_handle) error_handle();
    else;//打印日志：还没注册
}

void Chanel::CallDiscfunc()
{
    if(disconn_handle) disconn_handle();
    else;//打印日志：还没注册
}


