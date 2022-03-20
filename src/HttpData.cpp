// Created by llf on 2022/3/19.

#include "HttpData.h"

HttpData::HttpData(Chanel* CH,EventLoop* EV,Timer* TI):http_cha(CH),belong_sub(EV),http_timer(TI)
{
    main_state=main_State_ParseHTTP::check_state_requestline;
}

void HttpData::state_machine()
{
    bool stop=false,error=false;

    while(!stop && !error)
    {
        switch (main_state) {
            case check_state_requestline:
            {
                sub_state=parse_requestline();
                switch (sub_state) {
                    case requestline_data_is_not_complete:
                        return;
                    case requestline_parse_error:
                        Getlogger()->error("http requestline parse error, fd : {}", http_cha->Get_fd());
                        return;
                    case requestline_is_ok:
                        main_state = check_state_header;
                        break;
                }
            }break;

            case check_state_header:
            {
                sub_state=parse_header();
                switch (sub_state) {
                    case header_data_is_not_complete:
                        return;
                    case header_parse_error:
                        Getlogger()->error("http header parse error, fd:{}",http_cha->Get_fd());
                        return ;
                    case header_is_ok:
                        main_state=check_headerIsOk;
                        break;
                }
            }break;

            case check_headerIsOk:
            {
                //post要分析数据是否完整
                //get和head不用
            }break;

            case check_state_content:
            {

            }break;

        }
    }

    write_and_send();
}

sub_state_ParseHTTP HttpData::parse_requestline()
{

}

sub_state_ParseHTTP HttpData::parse_header()
{

}

void write_and_send()
{

}
