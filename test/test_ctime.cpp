//
// Created by llf on 2022/3/24.
//

#include<iostream>
#include<string>
#include<ctime>
#include<chrono>

using namespace std;

int main()
{
//    struct tm* ptr;
//    time_t lt;
//    lt=time(NULL);
//    ptr= localtime(&lt);
//    char timebuffer[100];
//    strftime(timebuffer,100,"%a, %d %b %Y %H:%M:%S ",ptr);
//
//    string res(timebuffer);
//    cout<<res<<endl;

    chrono::seconds time(60);
    cout<<to_string(time.count())<<endl;
    return 0;
}
