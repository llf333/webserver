// Created by llf on 2022/3/20.

#include<regex>
#include<iostream>
#include<string>
using namespace std;

int main()
{
    auto deal_with_perline= [=](std::string str) ->bool {
        std::regex reg(R"(^(\S*)\:\s(.*)$)");
        std::smatch res;
        std::regex_match(str,res,reg);
        if(!res.empty())
        {
            cout<<res[1]<<"     "<<res[2]<<endl;
            return true;
        }
        else return false;
    };//处理每行数据，将数据的key和value存到map中

    unsigned long current_pos=0;
    string read_buffer=
            "\r\nHOST: www.XXX.com\r\nUser-Agent: Mozilla/5.0(Windows NT 6.1;rv:15.0) Firefox/15.0\r\n\r\n";

    while(true)
    {
        current_pos=read_buffer.find("\r\n",2);//请求行的\r\n没有删

        if(current_pos == std::string::npos)
        {
            cout<<"data is not complete"<<endl;
            return 0;
        }//没接收到完整数据

        if(current_pos == 2)
        {
            cout<<"header is ok"<<endl;
            return 0;
        }//搜索到空行了

        std::string newline=read_buffer.substr(2,current_pos-2);

        read_buffer=read_buffer.substr(current_pos);//删除已提取的数据，注意：没删除\r\n

        if(!deal_with_perline(newline))
        {
            cout<<"data is wrong"<<endl;
            return 0;
        }//格式出错

    }

    return 0;
}



