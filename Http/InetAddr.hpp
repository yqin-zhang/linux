#pragma once

// 这个类，描述client socket信息的类
// 方便我们后续用它来管理客户端

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

#define Conv(addr) ((struct sockaddr *)&addr)

class InetAddr
{
private:
    void Net2Host()
    {
        _port = ntohs(_addr.sin_port);
        //_ip = inet_ntoa(_addr.sin_addr);
        char ipbuffer[64];
        _ip = inet_ntop(AF_INET, &(_addr.sin_addr.s_addr), ipbuffer, sizeof(ipbuffer));
        _ip = ipbuffer;
    }
    void Host2Net()
    {

        memset(&_addr, 0, sizeof(_addr));
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(_port);
        inet_pton(AF_INET, _ip.c_str(), &_addr.sin_addr.s_addr);
    }

public:
    InetAddr()
    {
    }
    InetAddr(const struct sockaddr_in &addr)
        : _addr(addr)
    {
        Net2Host();
    }

    InetAddr(uint16_t port, const std::string &ip = "0.0.0.0")
        : _port(port),
          _ip(ip)

    {
        Host2Net();
    }
    void ReInit(const struct sockaddr_in &addr)
    {
        _addr = addr;
        Host2Net();
    }
    std::string Ip()
    {
        return _ip;
    }
    uint16_t Port()
    {
        return _port;
    }
    struct sockaddr *Addr()
    {
        return Conv(_addr);
    }
    socklen_t length()
    {
        return sizeof(_addr);
    }
    std::string ToString()
    {
        return _ip + "-" + std::to_string(_port);
    }
    bool operator==(const InetAddr &addr)
    {
        return (_ip == addr._ip);                        // 多个客户端
        return (_ip == addr._ip && _port == addr._port); // 一个客户端
    }

    ~InetAddr()
    {
    }

private:
    struct sockaddr_in _addr; // 网络地址
    // 主机风格地址
    std::string _ip;
    uint16_t _port;
};
