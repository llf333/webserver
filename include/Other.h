// Created by llf on 2022/3/11.
#ifndef WEBSERVER_OTHER_H
#define WEBSERVER_OTHER_H

#include<chrono>
#include<unistd.h>
#include<mutex>
#include<cstring>
#include<optional>
#include<string>
#include<regex>
#include<iostream>
#include<ctime>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

class GlobalValue
{
public:
    static const int TheMaxConnNumber=100000;               //最大连接数
    static int CurrentUserNumber;                           //总连接数
    static int BufferMaxSize;                               //读数据时buffer的最大长度

    static std::chrono::seconds HttpHEADTime;               //tcp连接建立后,必须在该时间内接收到完整的请求行和首部行，否则超时
    static std::chrono::seconds HttpPostBodyTime;           //Post报文，相邻报文到达时间的最大间隔
    static std::chrono::seconds keep_alive_time;            //长连接的超时时间
    static int TimeWheel_PerSlotTime;                       //时间轮一个槽代表的时间
    static char Favicon[555];

private:
    static std::mutex usernumber_mtx;
public:
    static void Inc_Current_user_number()
    {
        std::unique_lock<std::mutex> locker(usernumber_mtx);
        CurrentUserNumber++;
    }
    static void Dec_Current_user_number()
    {
        std::unique_lock<std::mutex> locker(usernumber_mtx);
        CurrentUserNumber--;
    }

    static int GetUserNUmber()
    {
        std::unique_lock<std::mutex> locker(usernumber_mtx);
        return CurrentUserNumber;
    }

};


/*!
@brief ET模式下从连接socket读取数据。

@param[in] fd           连接socket的文件描述符。
@param[in] buffer       存放数据的string，赋值。
@param[in] is_disconn   true表示客户端已断开连接，false表示客户端未断开连接，赋值。
@return    成功读取的字节数，可能小于n。-1表示数据读取出错。
*/
int ReadData(int fd,std::string &buffer,bool &is_disconn);


/*!
@brief ET模式下向连接socket写入数据并删除buffer中成功写出的数据。

@param[in] fd      连接socket的文件描述符。
@param[in] buffer  待写的数据。
@param[in] full    true表示写数据失败是由于输出缓冲区已满造成的。
@return    成功写出的字节数，可能小于n。-1表示写数据出错。
*/
int WriteData(int fd,std::string& buffer,bool& full);

/*!
@brief ET模式下向文件描述符(非连接socket)写n个字节的数据。

@param[in] fd      文件描述符。
@param[in] content  待写数据的首地址。
@param[in] length       待写数据的字节数。
@return    成功写出的字节数，可能小于n。-1表示写数据出错。
*/
int Write_to_fd(int fd,const char* content,int length);

/*!
@brief ET模式下从文件描述符(非连接socket)读n个字节的数据。

@param[in] fd     文件描述符。
@param[in] buffer   数据存放的首地址。
@param[in] length     期望读取的字节数。
@return    成功读取的字节数，可能小于n。-1表示数据读取出错。
*/
int Read_from_fd(int fd,const char* buffer,int length);

/*!
 * 生成全局唯一的logger对象
 * @param path
 * @return
 */
std::shared_ptr<spdlog::logger> Getlogger(std::string path="./log/log.txt");

int BindAndListen(int pot);                                //绑定端口号并监听。成功时返回监听socket的文件描述符，否则返回-1。全连接队列长度设为2048

int setnonblocking(int fd);                                //将文件描述符fd设置为非阻塞模式。

std::string GetTime();                                     //获取当前时间并按一定的格式输出

std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[]);//C++ 17 解析命令行选项

#endif //WEBSERVER_OTHER_H
