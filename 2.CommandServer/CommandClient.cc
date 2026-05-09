#include <iostream>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include"InetAddr.hpp"
#include"Comm.hpp"
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage: " << argv[0] << " serverip serverport" << std::endl;
        exit(0);
    }
    
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);
    
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
        std::cout<<"error"<<std::endl;
        exit(SOCK_CREATE_ERR);
    }
    //TCP客户端，要不要显示的bind？不能。要不要"bind"？一定要
    //客户端自己的socket地址，让本地OS自主随机选择
    //向目标服务器发起连接请求

    InetAddr server(serverport,serverip);

    if(connect(sockfd,server.Addr(),server.length())!=0)

    {
        std::cerr<<"connect server error"<<std::endl;
        exit(SOCK_CONNECT_ERR);
    }
    std::cout<<"connect"<<server.Ip()<<"-"<<server.Port()<<"success"<<std::endl;

    while(1)
    {
        std::cout<<"Please Enter@ ";
        std::string line;

        std::getline(std::cin,line);

        ssize_t n=write(sockfd,line.c_str(),line.size());
        if(n>=0)
        {
            char buffer[1024];
            ssize_t m=read(sockfd,buffer,sizeof(buffer)-1);
            if(m>0)
            {
                buffer[m]=0;
                std::cout<<""<<std::endl;
            }
        }
    }
    
    return 0;
}