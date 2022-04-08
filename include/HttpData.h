// Created by llf on 2022/3/19.
#ifndef WEBSERVER_HTTPDATA_H
#define WEBSERVER_HTTPDATA_H

#include<string>
#include<map>
#include<regex>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/mman.h>
#include<unordered_map>

#include"Chanel.h"
#include"Other.h"

class Chanel;
class EventLoop;
class Timer;

// 文件类型映射
class SourceMap
{
private:
    static void Init();                                                                  //每个线程只调用一次
    SourceMap() =default;

public:
    SourceMap(SourceMap& other) =delete;
    SourceMap operator=(SourceMap& other) =delete;
    static std::string Get_file_type(std::string file_type);                             //根据后缀获取文件类型

private:
    static std::unordered_map<std::string,std::string> source_map;                       //文件后缀与文件类型的映射
    static std::once_flag o_flag;                                                        //配合std::call_once函数保证Init()在多个线程中只被调用一次
};

//主状态机
enum main_State_ParseHTTP{check_state_requestline, check_state_header, check_headerIsOk, check_body, check_state_analyse_content};

//从状态机
enum sub_state_ParseHTTP{
    requestline_data_is_not_complete, requestline_parse_error, requestline_is_ok,       //这一行表示的是解析请求行的状态
    header_data_is_not_complete, header_parse_error, header_is_ok,                      //这一行表示的是解析首部行的状态
    analyse_error,analyse_success                                                       //分析报文并填充发送报文状态
};

class HttpData
{
private:
    Chanel* http_cha=nullptr;                                                                   //从属的事件
    EventLoop* belong_sub;                                                              //从属的Reactor
    Timer* http_timer{};                                                                  //绑定的事件器
                                                          //是否处于连接状态

    std::string write_buffer;                                                           //发送缓冲区
    std::string read_buffer;                                                            //接收缓冲区

    main_State_ParseHTTP main_state;                                                    //主状态
    sub_state_ParseHTTP sub_state;                                                      //从状态

    std::map<std::string,std::string> mp;                                               //存http请求报文信息，第一个参数为类型，第二个参数为具体内容。例如：mp["url"]="www.baidu.com";

public:
    HttpData(Chanel* CH,EventLoop* EV);
    ~HttpData();
    void state_machine();                                                               //状态机

    Timer* Get_timer(){return http_timer;}
    void Set_timer(Timer* timer_) {http_timer=timer_;}                                  //设置定时器


    void TimerTimeoutCallback();                                                        //定时器超时回调函数

private:
    sub_state_ParseHTTP parse_requestline();                                            //解析请求行
    sub_state_ParseHTTP parse_header();                                                 //解析首部行
    void Set_HttpErrorMessage(int fd,int erro_num,std::string msg);                     //设置错误报文

    sub_state_ParseHTTP Analyse_GetOrHead();                                            //分析报文并填充发送报文
    sub_state_ParseHTTP Analyse_Post();                                                 //分析报文并填充发送报文

    void Write_Response_GeneralData();                                                  //填充报文通用部分，包括：版本、结果、Server和连接状态

    void Reset_Http_events(bool in);                                                    //重新注册Chanel事件
    void Reset();                                                                       //重置

    //四种回调函数
    void call_back_in();                                                                //读
    void call_back_out();                                                               //写
    void call_back_error();                                                             //错误
    void call_back_rdhub();                                                             //断开连接

};

#endif //WEBSERVER_HTTPDATA_H
