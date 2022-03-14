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
};



bool ReadData(int fd,char* buffer);

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
