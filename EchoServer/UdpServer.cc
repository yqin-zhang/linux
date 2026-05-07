#include "UdpServer.hpp"
#include<iostream>
#include<memory>
#include "Logger.hpp"  // 注意命名空间

void Usage(std::string proc)
{
    std::cout<<"Usage:"<<proc<<"localprot"<<std::endl;
}
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


   // 启用控制台日志输出
    ENABLE_CONSOLE_LOG_STRATEGY();
    std::unique_ptr<UdpServer> usvr=std::make_unique<UdpServer>(port);
    usvr->Init();
    usvr->start();
}