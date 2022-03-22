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
                        error=true;
                        break;
                    case header_is_ok:
                        main_state=check_headerIsOk;
                        break;
                }
            }break;

            case check_headerIsOk:
            {
                //post要判断数据是否完整
                //get和head不用
                if(mp["method"]=="POST" || mp["method"]=="post") main_state=check_body;
                else main_state=check_state_analyse_content;
            }break;

            case check_body:
            {
                //前后两个报文时间不能超过xxx，因此该mod timer
                belong_sub->get_theTimeWheel()->TimerWheel_Adjust_Timer(http_timer,GlobalValue::HttpPostBodyTime);
                if(mp.count("Content-length"))//如果找到了content-length字段
                {

                }
                else//没找到表示出错了，因为content-length字段是在header中
                {

                }

            }

            case check_state_analyse_content:
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
    read_buffer=read_buffer.substr(pos);//删除已提取的数据，注意：没删除\r\n

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
    auto deal_with_perline= [=](std::string str) ->bool {
        std::regex reg(R"(^(\S*)\:\s(.*)$)");
        std::smatch res;
        std::regex_match(str,res,reg);

        if(!res.empty())
        {
            mp[res[1]]=res[2];
            return true;
        }
        else return false;

    };//处理每行数据，将数据的key和value存到map中

    unsigned long current_pos=0;
    while(true)
    {
        current_pos;
        current_pos=read_buffer.find("\r\n",2);//请求行的\r\n没有删
        if(current_pos == std::string::npos) return header_data_is_not_complete;//没接收到完整数据
        if(current_pos ==  2) return header_is_ok;//搜索到空行了

        std::string newline=read_buffer.substr(2,current_pos-2);

        read_buffer=read_buffer.substr(current_pos);//删除已提取的数据，注意：没删除\r\n

        if(!deal_with_perline(newline)) return header_parse_error;//格式出错
    }
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
