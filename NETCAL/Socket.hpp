#ifndef __SOCKET__HPP__
#define __SOCKET__HPP__

#include <iostream>
#include <string>
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
    virtual ~Socket()
    {
    }
    virtual void CreateSocketOrDie() = 0;
    virtual void BindSocketOrDie(int port) = 0;
    virtual void ListenSocketOrDie(int backlog) = 0;
    virtual std::shared_ptr<Socket> Accept(InetAddr *clientaddr) = 0;
    virtual int Sockfd();
    virtual void Close();

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
    TcpSocket()
    {
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
            LOG(LogLevel::WARNING) << "accpet error";
            return nullptr;
        }
        LOG(LogLevel::INFO) << "accpet success";

        clientaddr->ReInit(peer);
        return std::make_shared<TcpServer>();
    }
    int Sockfd() override
    {
        return _sockfd;
    }
    void Close() override
    {
        close(_sockfd);
    }
    ~TcpSocket()
    {
    }

private:
    int _sockfd;
};

#endif