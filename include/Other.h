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

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"

class GlobalValue
{
public:
    static std::chrono::seconds client_header_timeout;
    static int TheMaxConnNumber;
    static int CurrentUserNumber;
    static int BufferMaxSize;

    static std::chrono::seconds HttpConnectTime;

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

/*!
 * 生成全局唯一的logger对象
 * @param path
 * @return
 */
std::shared_ptr<spdlog::logger> Getlogger(std::string path="./log/log.txt");

int BindAndListen(int pot);

int setnonblocking(int fd);

std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[]);//C++ 17

#endif //WEBSERVER_OTHER_H
