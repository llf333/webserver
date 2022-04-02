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
    static std::chrono::seconds client_header_timeout;

    static int TheMaxConnNumber;
    static int CurrentUserNumber;
    static int BufferMaxSize;

    static std::chrono::seconds HttpHEADTime;
    static std::chrono::seconds HttpPostBodyTime;
    static std::chrono::seconds keep_alive_time;
    static int TimeWheel_PerSlotTime;

private:
    static std::mutex usernumber_mtx;
public:
    static void Inc_Current_user_number()
    {
        std::unique_lock<std::mutex> locker(usernumber_mtx);
        CurrentUserNumber++;
    }
};


int ReadData(int fd,std::string &buffer,bool &is_disconn);//返回读了多少数据
int WriteData(int fd,std::string& buffer,bool& full);//返回写了多少数据，并随时判断发送缓冲区是否已满
int Write_to_fd(int fd,const char* content,int length);

/*!
 * 生成全局唯一的logger对象
 * @param path
 * @return
 */
std::shared_ptr<spdlog::logger> Getlogger(std::string path="./log/log.txt");

int BindAndListen(int pot);

int setnonblocking(int fd);

std::string GetTime();


std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[]);//C++ 17

#endif //WEBSERVER_OTHER_H
