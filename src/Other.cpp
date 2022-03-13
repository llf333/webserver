// Created by llf on 2022/3/11.

#include "Other.h"

///////////////
//Global Value//
///////////////

std::chrono::seconds GlobalValue::client_header_timeout=std::chrono::seconds(60);
bool ReadData(int fd,char* buffer);


std::optional<std::tuple<int,int,std::string>> AnalyseCommandLine(int argc,char* argv[])
{
    const char* str="p:s:l:";
    int res;
    int port,subReactorSize;
    std::string log_path;
    while((res= getopt(argc,argv,str))!=-1)
    {
        switch (res) {
            case 'p':
                port = atoi(optarg);
                break;
            case 's':
                subReactorSize = atoi(optarg);
                break;
            case 'l': {
                std::regex ExpressOfPath(R"(^\.\/\S*)");//llf start with ./
                std::smatch res{};
                std::string path(optarg);
                std::regex_match(path, res, ExpressOfPath);
                if (res.empty()) {
                    std::cout << "illegal log path" << std::endl;
                    return std::nullopt;
                }
                log_path = res[0];
            }
                break;
            default:
                break;
        }
    }
    return std::tuple<int,int,std::string>(port,subReactorSize,log_path);
}

bool ReadData(int fd,char* buffer)
{
    return true;
}
