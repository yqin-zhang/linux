#ifndef __UDP_SERVER_HPP__
#define __UDP_SERVER_HPP__

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <cstdlib>
#include "Logger.hpp"

using namespace LogModule;

static const int gdefaultsockfd = -1;

class UdpServer
{
public:
    UdpServer(uint16_t port)
        : _port(port),
          _sockfd(gdefaultsockfd),
          _isrunning(false)
    {
        // 构造函数，可以初始化其他成员
    }
    
    ~UdpServer()
    {
        if(_sockfd != gdefaultsockfd)
        {
            close(_sockfd);
        }
    }
    
    void Init()
    {
        // 1. 创建socket fd套接字文件描述符
        // 把网络在文件系统中打开
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(_sockfd < 0)
        {
            LOG(LogLevel::FATAL) << "create socket err";
            exit(1);
        }
        LOG(LogLevel::INFO) << "create socket success, sockfd: " << _sockfd;
        
        // 2. bind
        // 2.1. 填充IP和Port
        // 我们有没有实现，把socket和file关联起来？没有！
        struct sockaddr_in local;
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(_port);  // 注意：用 htons，不是 htonl
        // local.sin_addr.s_addr = inet_addr(_ip.c_str());
        // local.sin_addr.s_addr = INADDR_ANY; // 任意IP
        // 什么叫做任意IP bind? 不明确具体IP，只要是发给本机的消息都能收到
        local.sin_addr.s_addr = INADDR_ANY;  // INADDR_ANY 已经是网络字节序，不需要再 htonl
        
        // 2.2 和socketfd进行bind
        int n = bind(_sockfd, (struct sockaddr*)&local, sizeof(local));
        if(n < 0)
        {
            LOG(LogLevel::FATAL) << "bind socket error";
            exit(2);
        }
        LOG(LogLevel::INFO) << "bind socket success, port: " << _port;
    }
    
    void start()
    {
        _isrunning = true;
        char buffer[1024];
        
        while(_isrunning)
        {
            buffer[0] = 0;
            struct sockaddr_in peer;
            socklen_t peer_len = sizeof(peer);
            
            // 接收客户端消息
            ssize_t n = recvfrom(_sockfd, buffer, sizeof(buffer) - 1, 0,
                                 (struct sockaddr*)&peer, &peer_len);
            
            if(n > 0)
            {
            

                buffer[n] = '\0';
                
                // 获取客户端信息
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &peer.sin_addr, client_ip, sizeof(client_ip));
                uint16_t client_port = ntohs(peer.sin_port);
                
                LOG(LogLevel::INFO) << "收到来自 " << client_ip 
                                    << ":" << client_port 
                                    << " 的消息: " << buffer;
                
                // 回声：原样发回给客户端
                sendto(_sockfd, buffer, n, 0,
                       (struct sockaddr*)&peer, peer_len);
            }
        }
    }
    
    void Stop()
    {
        _isrunning = false;
    }
    
private:
    // std::string _ip;  // 如果需要绑定特定IP，取消注释
    uint16_t _port;
    int _sockfd;
    bool _isrunning;
};

#endif // __UDP_SERVER_HPP__