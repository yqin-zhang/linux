#ifndef __TCP_ECHO_SERVER_HPP__
#define __TCP_ECHO_SERVER_HPP__

#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> //sockaddr_in
#include <cstdlib>      //exit
#include "Comm.hpp"
#include "Logger.hpp"

using namespace LogModule;

static const int gdefaultfd = -1;
static const int gbacklog = 8;
static const int gport = 8080;

class TcpEchoServer
{
public:
    // 失败返回的-1，默认为-1
    TcpEchoServer(int sockfd = gdefaultfd)
        : _listensockfd(sockfd)
    {
    }

    void Init()
    {
        // 1.创建套接字文件描述符
        // 失败-1
        _listensockfd = socket(AF_INET, SOCK_STREAM, 0);
        // 失败
        if (_listensockfd < 0)
        {
            // 打印日志信息
            LOG(LogLevel::FATAL) << "create tcp socket error";
            // 退出码有要求
            exit(SOCK_CREATE_ERR);
        }
        LOG(LogLevel::INFO) << "create tcp socket success" << _listensockfd;

        // 2.bind socket ip port
        struct sockaddr_in local;
        memset(&local, 0, std::string::npos); // 清空
        // 协议家族
        local.sin_family = AF_INET;
        // 主机字节序列转网络字节序列
        local.sin_port = htons(_port);
        // 这里的addr是结构体，包括填充，但是这部分不用
        // 点分十进制转为二进制，进入网络
        // 前面UDP用的inet_addr,这个已经过时
        // 不能绑定具体IP
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        int n = bind(_listensockfd, (struct sockaddr *)&local, sizeof(local));
        // 因为成功返回0，失败返回-1
        if (n != 0) // 不成功
        {
            // 打印日志
            LOG(LogLevel::FATAL) << "bind socket error";
            // 退出码
            exit(SOCK_BIND_ERR);
        }
        LOG(LogLevel::INFO) << "bind tcp socket success" << _listensockfd;

        // 3.set socket listen
        if (listen(_listensockfd, gbacklog) != 0)
        {
            LOG(LogLevel::FATAL) << "listen socket error";
            // 退出码
            exit(SOCK_LISTEN_ERR);
        }
        LOG(LogLevel::INFO) << "listen socket success" << _listensockfd;
    }

    void Start()
    {
        // 死循环
        while (1)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(len);
            int sockfd = accept(_listensockfd, (struct sockaddr *)&peer, &len);

            if (sockfd < 0)
            {
                LOG(LogLevel::WARNING)<<"accept client error";
                continue;
            }
            LOG(LogLevel::INFO)<<"获取新连接成功:"<<sockfd;
        }
    }
    ~TcpEchoServer()
    {
    }

private:
    int _listensockfd; // 监听套接字

    uint16_t _port; // 2字节
};

#endif