#include "DictServer.hpp"//软件IO
#include"Dictionary.hpp"//翻译
#include<iostream>
#include<memory>
#include "Logger.hpp"  // 注意命名空间

void Usage(std::string proc)
{
    std::cout<<"Usage:"<<proc<<"localprot"<<std::endl;
}

// std::string translate(const std::string &word,const std::string &whoip,uint16_t whoport)
// {
//     return "hh";
// }

// ./udp_server serverip serverport
int main(int argc,char*argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(0);
    }
    //std::string ip=argv[1];
    uint16_t port=std::stoi(argv[1]);//字符串转整数

    Dictionary dict("./dict.txt");

   // 启用控制台日志输出
    ENABLE_CONSOLE_LOG_STRATEGY();
    std::unique_ptr<DictServer> usvr=std::make_unique<DictServer>(port,[&dict](const std::string &word,const std::string &whoip,uint16_t whoport){
        return dict.translate(word,whoip,whoport);
    });
    usvr->Init();
    usvr->start();
}