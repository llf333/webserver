// Created by llf on 2022/3/19.

#include "HttpData.h"


HttpData::HttpData(Chanel* CH,EventLoop* EV):http_cha(CH),belong_sub(EV)
{
    main_state=main_State_ParseHTTP::check_state_requestline;

    //注册chanel的回调函数
    http_cha->Register_RdHandle([=](){call_back_in();});
    http_cha->Register_WrHandle([=](){call_back_out();});
    http_cha->Register_ErHandle([=](){call_back_error();});
    http_cha->Register_DiscHandle([=](){call_back_rdhub();});
}

HttpData::~HttpData()
{

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
                        return ;
                    case requestline_parse_error:
                        Getlogger()->error("http requestline parse error, fd : {}", http_cha->Get_fd());
                        error=true;
                        break;
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

    write_and_send(error);

}

sub_state_ParseHTTP HttpData::parse_requestline()
{
    std::string requestline{};
    auto pos=read_buffer.find("\r\n");

    if(pos == std::string::npos) return sub_state_ParseHTTP::requestline_data_is_not_complete;

    requestline=read_buffer.substr(0,pos);

    std::regex reg(R"(^(POST|GET|HEAD|pos|get|head)\s(\S*)\s((HTTP|http)\/\d\.\d)$)");
    std::smatch res;
    std::regex_match(requestline,res,reg);

    if(!res.empty())
    {
        mp["method"]=res[1];
        mp["URL"]=res[2];
        mp["version"]=res[3];

        return sub_state_ParseHTTP::requestline_is_ok;
    }

    return sub_state_ParseHTTP::requestline_parse_error;
}

sub_state_ParseHTTP HttpData::parse_header()
{

}

void HttpData::write_and_send(bool error)
{

}

void HttpData::call_back_in()
{
    //读数据到buffer中，然后利用状态机解析
    int fd=http_cha->Get_fd();

    if(ReadData(fd,read_buffer,dis_conn)<=0 || dis_conn)
    {
        Getlogger()->error("failed to read the data from the fd{} (http fd)",fd);

        if(dis_conn)
        {
            Getlogger()->error("the http socket is closed");
            call_back_rdhub();
        }

        return ;
    }

    state_machine();

}
