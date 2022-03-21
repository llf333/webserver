// Created by llf on 2022/3/19.
#ifndef WEBSERVER_HTTPDATA_H
#define WEBSERVER_HTTPDATA_H

#include<string>
#include<map>
#include<regex>

#include"Chanel.h"
#include"Other.h"

class Chanel;
class EventLoop;
class Timer;

//主状态机
enum main_State_ParseHTTP{check_state_requestline, check_state_header, check_headerIsOk, check_body, check_state_analyse_content};

//从状态机
enum sub_state_ParseHTTP{
    requestline_data_is_not_complete, requestline_parse_error, requestline_is_ok,//这一行表示的是解析请求行的状态
    header_data_is_not_complete, header_parse_error, header_is_ok,//这一行表示的是解析首部行的状态

};

class HttpData
{
private:
    //chanel 和 eventloop 在Server.cpp中绑定，时间器在往时间轮添加时间器时绑定
    Chanel* http_cha;
    EventLoop* belong_sub;
    Timer* http_timer;

    bool dis_conn=false;

    std::string write_buffer;
    std::string read_buffer;

    main_State_ParseHTTP main_state;
    sub_state_ParseHTTP sub_state;

    std::map<std::string,std::string> mp;//存http请求报文信息，第一个参数为类型，第二个参数为具体内容。例如：mp["url"]="www.baidu.com";

public:
    HttpData(Chanel* CH,EventLoop* EV);
    ~HttpData();
    void state_machine();

    void Set_timer(Timer* timer_) {http_timer=timer_;}//还要设置定时器的超时回调函数

    //定时器超时回调函数
    void TimerTimeoutCallback();

private:
    sub_state_ParseHTTP parse_requestline();
    sub_state_ParseHTTP parse_header();

    void write_and_send(bool error);

    //四种回调函数
    void call_back_in();
    void call_back_out();
    void call_back_error();
    void call_back_rdhub();




};









#endif //WEBSERVER_HTTPDATA_H
