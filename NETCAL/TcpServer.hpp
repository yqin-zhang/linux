#pragma once

#include "Socket.hpp"
#include <memory>
#include "InetAddr.hpp"
#include <unistd.h>
// 只负责IO通信
class TcpServer
{
public:
    TcpServer(int port)
        : _port(port),
          _listensocket(std::make_unique<TcpServer>())
    {
        _listensocket->BuildListenSocketMethod(_port);
    }
    void Run()
    {
        while (1)
        {
            signal()
                // 你要得到一个sockfd,client,addr
                InetAddr addr;
            auto sockfd = _listensocket->Accept(&addr);
            if (sockfd == nullptr)
            {
                continue;
            }
            LOG(LogLevel::DEBUG) << "获取一个新连接：" << addr.ToString() << ",sockfd:" << sockfd;

            // 进程
            if (fork() == 0)
            {
                _listensocket->Close();
            }
        }
    }
    ~TcpServer()
    {
    }

private:
    int _port;
    std::unique_ptr<Socket> _listensocket;
};