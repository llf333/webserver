// Created by llf on 2022/3/16.
#include<iostream>
#include<thread>

#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<assert.h>

int main(int argc,char* argv[])
{
    if(argc<=2)
    {
        std::cout<<"please input the PORT and the IP"<<std::endl;
        return -1;
    }

    const char* ip=argv[2];
    int port= atoi(argv[1]);

    struct sockaddr_in server_address;
    bzero(&server_address,sizeof server_address);
    server_address.sin_family=AF_INET;
    server_address.sin_port= htons(port);
    inet_pton(AF_INET,ip,&server_address.sin_addr);



    while(1)
    {
        int sockfd= socket(PF_INET,SOCK_STREAM,0);
        assert(sockfd>=0);
        int res=connect(sockfd,reinterpret_cast<sockaddr*>(&server_address),sizeof server_address);
        if(res==0)
        {
            std::cout<<"连接服务器成功"<<std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        else break;
    }

//    if(connect(sockfd,reinterpret_cast<sockaddr*>(&server_address),sizeof server_address)==0)
//    {
//        std::cout<<"连接服务器成功"<<std::endl;
//        std::this_thread::sleep_for(std::chrono::seconds(15));
//
//        return 0;
//
//    }

    std::cout<<"连接服务器失败"<<std::endl;
    return- -1;
}


