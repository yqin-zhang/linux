#ifndef __SOCKET__HPP__
#define __SOCKET__HPP__

#include <iostream>
#include <string>
#include <memory>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include "Logger.hpp"
#include "InetAddr.hpp"
using namespace LogModule;

enum
{
    OK,
    CREATE_ERR,
    BIND_ERR,
    LISTEN_ERR
};

static int gbacklog = 16;
static const int gsockfd = -1;

class Socket
{
public:
    virtual ~Socket() {}
    virtual void CreateSocketOrDie() = 0;
    virtual void BindSocketOrDie(int port) = 0;
    virtual void ListenSocketOrDie(int backlog) = 0;
    virtual std::shared_ptr<Socket> Accept(InetAddr *clientaddr) = 0;
    virtual int Sockfd() = 0;
    virtual void Close() = 0;
    virtual ssize_t Recv(std::string *out) = 0;
    virtual ssize_t Send(std::string *in) = 0;

public:
    void BuildListenSocketMethod(int _port)
    {
        CreateSocketOrDie();
        BindSocketOrDie(_port);
        ListenSocketOrDie(gbacklog);
    }
};

class TcpSocket : public Socket
{
public:
    TcpSocket()
        : _sockfd(gsockfd)
    {
    }

    ~TcpSocket() // 修改：将第二个构造函数改为析构函数
    {
        Close();
    }

    void CreateSocketOrDie() override
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            LOG(LogLevel::FATAL) << "create error";
            exit(CREATE_ERR);
        }
        LOG(LogLevel::INFO) << "create success";
    }

    void BindSocketOrDie(int port) override
    {
        InetAddr local(port);
        if (bind(_sockfd, local.Addr(), local.length()) != 0)
        {
            LOG(LogLevel::FATAL) << "bind error";
            exit(BIND_ERR);
        }
        LOG(LogLevel::INFO) << "bind success";
    }

    void ListenSocketOrDie(int backlog) override
    {
        if (listen(_sockfd, backlog) != 0)
        {
            LOG(LogLevel::FATAL) << "listen error";
            exit(LISTEN_ERR);
        }
        LOG(LogLevel::INFO) << "listen success";
    }

    std::shared_ptr<Socket> Accept(InetAddr *clientaddr) override
    {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        int fd = accept(_sockfd, (struct sockaddr *)&peer, &len);
        if (fd < 0)
        {
            LOG(LogLevel::WARNING) << "accept error";
            return nullptr;
        }
        LOG(LogLevel::INFO) << "accept success";

        clientaddr->ReInit(peer);
        auto sock = std::make_shared<TcpSocket>(); // 修改：创建TcpSocket对象
        sock->_sockfd = fd;
        return sock;
    }

    int Sockfd() override
    {
        return _sockfd;
    }

    void Close() override
    {
        if (_sockfd >= 0)
        {
            close(_sockfd);
            _sockfd = -1; // 修改：设置为-1避免重复关闭
        }
    }

    ssize_t Recv(std::string *out) override
    {
        char buffer[1024];
        ssize_t n = recv(_sockfd, buffer, sizeof(buffer) - 1, 0); // 修改：预留空间给'\0'
        if (n > 0)
        {
            buffer[n] = '\0';
            *out += buffer;
        }
        return n;
    }

    ssize_t Send(std::string *in) override // 修改：添加override关键字
    {
        return send(_sockfd, in->c_str(), in->size(), 0);
    }

private:
    int _sockfd;
};

#endif