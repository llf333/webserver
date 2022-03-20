// Created by llf on 2022/3/20.

#include<regex>
#include<iostream>
#include<string>
using namespace std;

int main()
{
    string target="POST /index.html HTTP/1.1";
    regex reg(R"(^(POST|GET|HEAD|pos|get|head)\s(\S*)\s((HTTP|http)\/\d\.\d)$)");
    smatch res;
    regex_match(target,res,reg);

    cout<<res[0]<<endl<<res[1]<<endl<<res[2]<<endl<<res[3]<<endl;
    return 0;
}



