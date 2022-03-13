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

class GlobalValue
{
public:
    static std::chrono::seconds client_header_timeout;
};



bool ReadData(int fd,char* buffer);

std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[]);

#endif //WEBSERVER_OTHER_H
