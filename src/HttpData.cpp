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
    bool finish=false,error=false;

    while(!finish && !error)
    {
        switch (main_state) {
            case check_state_requestline:
            {
                sub_state=parse_requestline();
                switch (sub_state) {
                    case requestline_data_is_not_complete:
                    {
                        std::cout<<"data is not complete"<<std::endl;
                        return ;//直接return表示不发送数据
                    }

                    case requestline_parse_error:
                        Set_HttpErrorMessage(http_cha->Get_fd(),400,"Bad Request: Request line has syntax error");
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
                        Set_HttpErrorMessage(http_cha->Get_fd(),400,"Bad Request: Header line has syntax error");
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

            case check_body://post报文会在请求头部中多Content-Type和Content-Length两个字段。
            {
                //前后两个报文时间不能超过xxx，因此该mod timer
                belong_sub->get_theTimeWheel()->TimerWheel_Adjust_Timer(http_timer,GlobalValue::HttpPostBodyTime);
                if(mp.count("Content-length")||mp.count("content-length"))//如果找到了content-length字段
                {
                    //找到了之后判断数据包的大小，因为在解析header时留下了一个/r/n，因此数据包的实际大小应该比数值大2
                    //如果小于，则证明还没接收完整，返回，等待下一次接收
                    std::string length="0";
                    if(mp.count("Content-length"))  length=mp["Content-length"];
                    if(mp.count("content-length"))  length=mp["content-length"];
                    int content_length=std::stoi(length);

                    if(read_buffer.size() < content_length+2) return ;//数据还没接收完整
                    else main_state=check_state_analyse_content;//接收完整则切换到分析状态
                }
                else//没找到表示出错了
                {
                    Set_HttpErrorMessage(http_cha->Get_fd(), 400, "Bad Request: Lack of argument (Content-Length)");
                    error=true;
                    break;
                }
            }break;

            case check_state_analyse_content://分析并填充发送报文
            {
                if(mp.count("method"))
                {
                    if(mp["method"]=="post" || mp["method"]=="POST")
                        sub_state=HttpData::Analyse_Post();
                    else if(mp["method"]=="get" || mp["method"]=="GET" || mp["method"]=="head" || mp["method"]=="HEAD")
                        sub_state=HttpData::Analyse_GetOrHead();
                    else return ;//其他方法暂时不做处理


                    if(sub_state == analyse_error)
                    {
                        Set_HttpErrorMessage(http_cha->Get_fd(),500,"Internal Server Error: analyse data not success");
                        error=true;
                        break;
                    }

                    if(sub_state == analyse_success)
                    {
                        finish=true;
                        break;
                    }
                }

                else
                {
                    Getlogger()->error("http data not found method");
                    return ;
                }
            }break;
        }
    }

    Http_send();
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

/*
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
*///解析首部行时，每解析一行就删除一行，不好（因为数据可能不完整）

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

    unsigned long current_pos=0,old_pos=0;//多设置了old指针
    while(true)
    {
        old_pos=current_pos;
        current_pos=read_buffer.find("\r\n",old_pos+2);//请求行的\r\n没有删
        if(current_pos == std::string::npos) return header_data_is_not_complete;//没接收到完整数据

        if(current_pos == old_pos+2)
        {
            read_buffer=read_buffer.substr(current_pos);//删除已提取的数据，注意：没删除空行中的\r\n
            return header_is_ok;//搜索到空行了
        }
        std::string newline=read_buffer.substr(old_pos+2,current_pos-2-old_pos);

        if(!deal_with_perline(newline)) return header_parse_error;//格式出错
    }
}

void HttpData::Http_send()
{
    int fd=http_cha->Get_fd();
    int data_sum=write_buffer.size()+1;
    int write_sum=0;
    bool full=false;//表示发送缓冲区是否已满

    while(write_sum<data_sum)
    {
         int ret=WriteData(fd,write_buffer,full);
         if(ret==-1)
         {
             //写数据出错，断开连接
             call_back_rdhub();
             return;
         }

         if( full && write_sum < data_sum)//如果发送缓冲区满了，但是数据还没发送完，则重新注册Epollout事件，等待下次读入。
         {
             //注册
             Reset_Http_events(false);
             //删除定时器，防止超时
             belong_sub->get_theTimeWheel()->TimerWheel_Remove_Timer(http_timer);
             //设置自己的定时器为空
             http_timer= nullptr;
             //返回
             return ;
         }
         write_sum+=ret;
    }
    //发送完数据要重新注册Epollin，
    Reset_Http_events(true);

    //重置
    Reset();

}

void HttpData::Reset_Http_events(bool in)//true 表示注册Epollin； false 表示注册Epollout
{
    __uint32_t evnt;
    if(in) evnt=EPOLLIN | EPOLLRDHUP | EPOLLERR;
    else evnt=EPOLLOUT | EPOLLRDHUP | EPOLLERR;

    http_cha->Set_events(evnt);
    belong_sub->MODChanel(http_cha,evnt);
    return ;
}

void HttpData::Reset()
{
    //如果是长连接，则不断开连接，并更改超时时间
    if(mp["Connection"]=="keep-alive" || mp["Connection"]=="Keep-Alive")
    {
        auto timeout=GlobalValue::keep_alive_time;
        if(!Get_timer()) belong_sub->get_theTimeWheel()->TimeWheel_insert_Timer(timeout,this);
        else belong_sub->get_theTimeWheel()->TimerWheel_Adjust_Timer(Get_timer(),timeout);
    }
    else
    {
        call_back_rdhub();
        return ;
    }

    read_buffer.clear();
    write_buffer.clear();
    mp.clear();
    main_state=main_State_ParseHTTP::check_state_requestline;

}

void HttpData::Set_HttpErrorMessage(int fd,int erro_num,std::string msg)
{
    Getlogger()->error("Http erro {} from {},the msg is {}:",erro_num,fd,msg);
    //设置发送缓冲区的数据，不发送，由Http_send函数发送

    //编写body*************暂时还不知道为啥是这个格式
    std::string  response_body{};
    response_body += "<html><title>错误</title>";
    response_body += "<body bgcolor=\"ffffff\">";
    response_body += std::to_string(erro_num)+msg;
    response_body += "<hr><em> Hust---LLF Server</em>\n</body></html>\r\n";

    //编写header
    std::string response_header{};
    response_header += "Http/1.1 " + std::to_string(erro_num) + " " + msg + "\r\n";
    response_header += "Date: " + GetTime() + "\r\n";
    response_header += "Server: Hust---LLF\r\n";
    response_header += "Content-Type: text/html\r\n";
    response_header += "Content-length: " + std::to_string(response_body.size())+"\r\n";
    response_header += "\r\n";

    write_buffer=response_body+response_header;
    return ;
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

void HttpData::call_back_out()
{

}

void HttpData::call_back_error()
{

}

void HttpData::call_back_rdhub()
{

}

void HttpData::TimerTimeoutCallback()
{

}

sub_state_ParseHTTP HttpData::Analyse_GetOrHead()
{
    Write_Response_GeneralData();

    std::string filename=mp["URL"].substr(mp["URL"].find_last_of('\\')+1);//最后一个斜杠之后的为文件名(注意，这里不包括斜杠)

    //simple test
    //-----------

    //content-type字段
    std::string content_type=SourceMap::Get_file_type(filename.substr(filename.find_last_of('.')));//根据文件名中的后缀写出字段
    write_buffer += "content-type: " + content_type +"\r\n" ;

    //content-length字段
    std::string file_position="../resource/" + filename;
    struct stat file_information;
    if(stat(file_position.c_str(),&file_information)<0)
    {
        Set_HttpErrorMessage(http_cha->Get_fd(),404,"not found");
        Getlogger()->debug("fd {} not found {},404",http_cha->Get_fd(),filename);
        return analyse_error;
    }
    int lenth= file_information.st_size;
    write_buffer += "content-length: " + std::to_string(lenth) + "\r\n";

    //首部行结束
    write_buffer += "\r\n";

    //Head不需要实体
    if(mp["method"]=="HEAD" || mp["method"]=="head" ) return analyse_success;

    //填充Get方法的实体报文
    int file_fd=open(file_position.c_str(),O_RDONLY);

    void* addr=mmap(0,lenth,PROT_READ,MAP_PRIVATE,file_fd,0);
    close(file_fd);

    if(addr==(void*) -1)
    {
        munmap(addr,lenth);
        Set_HttpErrorMessage(http_cha->Get_fd(), 404, "not found");
        return analyse_error;
    }

    char* buffer=(char* ) addr;
    write_buffer += std::string(buffer,lenth);
    munmap(addr,lenth);
    return analyse_success;
}

sub_state_ParseHTTP HttpData::Analyse_Post()
{
    Write_Response_GeneralData();

    //这里只简单地将读到的数据转换为大写
    //read_buffer中起始是一个空行
    std::string body=read_buffer.substr(2);
    for(int i=0;i<body.size();i++)
        body[i]=std::toupper(body[i]);

    write_buffer += std::string("Content-Type: text/plain\r\n") + "Content-Length: " + std::to_string(body.size()) + "\r\n";
    write_buffer += "\r\n" + body;

    return analyse_success;

}

void HttpData::Write_Response_GeneralData()
{
    std::string status_line = mp["version"] + " 200 OK\r\n";
    std::string header_line{};
    header_line += "Date: "+ GetTime() + "\r\n";
    header_line += "Server: Hust---LLF\r\n";

    if(mp["Connection"]=="Keep-Alive" || mp["Connection"]=="keep-alive")
    {
        header_line += "Connection: keep-alive\r\n"
                + std::string("Keep-Alive: ") + std::string(std::to_string(GlobalValue::keep_alive_time.count())) +"s \r\n";

    }
    else
    {
        header_line += "Connection: Connection is closed\r\n";
    }

    write_buffer = status_line + header_line;
    return ;
}

//文件类型映射

std::unordered_map<std::string,std::string> SourceMap::source_map{};
std::once_flag SourceMap::o_flag{};
void SourceMap::Init()
{
    /*文件类型*/
    source_map[".html"] = "text/html";
    source_map[".avi"] = "video/x-msvideo";
    source_map[".bmp"] = "image/bmp";
    source_map[".c"] = "text/plain";
    source_map[".doc"] = "application/msword";
    source_map[".gif"] = "image/gif";
    source_map[".gz"] = "application/x-gzip";
    source_map[".htm"] = "text/html";
    source_map[".ico"] = "image/x-icon";
    source_map[".jpg"] = "image/jpeg";
    source_map[".png"] = "image/png";
    source_map[".txt"] = "text/plain";
    source_map[".mp3"] = "audio/mp3";
    source_map["default"] = "text/html";
}

std::string SourceMap::Get_file_type(std::string file_type)
{
    std::call_once(o_flag,Init);

    if(source_map.count(file_type)) return source_map[file_type];
    else return source_map["default"];
}
