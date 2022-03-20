// Created by llf on 2022/3/19.
#ifndef WEBSERVER_HTTPDATA_H
#define WEBSERVER_HTTPDATA_H

#include<string>
#include"Chanel.h"
#include"Other.h"

class Chanel;
class EventLoop;
class Timer;

enum main_State_ParseHTTP{check_state_requestline, check_state_header, check_headerIsOk, check_body, check_state_content};
enum sub_state_ParseHTTP{
    requestline_data_is_not_complete, requestline_parse_error, requestline_is_ok,
    header_data_is_not_complete, header_parse_error, header_is_ok,

};

class HttpData
{
private:
    Chanel* http_cha;
    EventLoop* belong_sub;
    Timer* http_timer;

    std::string write_buffer;
    std::string read_buffer;

    main_State_ParseHTTP main_state;
    sub_state_ParseHTTP sub_state;

public:
    HttpData(Chanel* CH,EventLoop* EV,Timer* TI);
    ~HttpData();
    void state_machine();
private:
    sub_state_ParseHTTP parse_requestline();
    sub_state_ParseHTTP parse_header();

    void write_and_send();


};









#endif //WEBSERVER_HTTPDATA_H
