// Created by llf on 2022/3/11.
#ifndef WEBSERVER_OTHER_H
#define WEBSERVER_OTHER_H

#include<chrono>
#include<unistd.h>
#include<mutex>
#include<cstring>


class GlobalValue
{
public:
    static std::chrono::seconds client_header_timeout;
};



bool ReadData(int fd,char* buffer);

#endif //WEBSERVER_OTHER_H
