// Created by llf on 2022/3/27.

#include<iostream>
#include<string>
#include<string.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<unistd.h>

using namespace std;

int main(int argc,char* argv[])//格式./test port 127.0.0.1
{
    if(argc<2)
    {
        cout<<"too few value"<<endl;
        return 0;
    }

    //下面写http报文
    string head{};
    string get{};
    string post{};

    //head
    head += "HEAD /index.html HTTP/1.1\r\n";
    head += "Connection: keep-alive\r\n";
    head += "\r\n";

    //get
    get += "GET /index.html HTTP/1.1\r\n";
    get += "Connection: close\r\n";
    get += "\r\n";

    //post
    post += "POST / HTTP/1.1\r\n";
    post += "Connection: keep-alive\r\n";
    post += "Content-Length: 5\r\n";
    post += "Content-Type: text/plain\r\n";
    post += "\r\nabcdf";


    int sockfd= socket(PF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        cout<<"create socket error"<<endl;
        return 0;
    }

    int port= atoi(argv[1]);
    const char* ip=argv[2];

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof serveraddr);

    serveraddr.sin_port= htons(port);
    serveraddr.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&serveraddr.sin_addr);

    if(connect(sockfd,reinterpret_cast<struct sockaddr*>(&serveraddr),sizeof serveraddr) <0 )
    {
        cout<<"connect error"<<endl;
        return 0;
    }

   // sleep(5);

    //发送数据
    const char* buf=post.c_str();
    cout<<buf<<endl;
    int rest=send(sockfd,buf, strlen(buf),0);//bug————————————————之前用的sizeof，sizeof char* 为8个字节！！！！！！
    if(rest<0)
    {
        cout<<"send data error"<<endl;
        return 0;
    }
    cout<<rest<<endl;

  //  close(sockfd);
    sleep(5);

    char rec_buf[1024];

    //疑问：需要阻塞吗
        memset(rec_buf,'\0',sizeof rec_buf);
        int ret= recv(sockfd,rec_buf,1024,0);
        if(ret<=0)
        {
            cout<<"recv ret<=0"<<endl;
        }
        if(ret>0) cout<<rec_buf<<endl;


    return 0;
}
