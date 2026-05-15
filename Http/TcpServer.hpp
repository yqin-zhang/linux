#pragma once

#include "Socket.hpp"
#include <memory>
#include "InetAddr.hpp"
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <functional>
// 回调。输入输出
using callback_t = std::function<std::string(std::string &)>;

// 只负责IO通信
class TcpServer
{
public:
    TcpServer(uint16_t port, callback_t cb)
        : _port(port),
          _listensocket(std::make_unique<TcpSocket>()), // 修改：创建TcpSocket对象
          _cb(cb)
    {
        _listensocket->BuildListenSocketMethod(_port);
    }

    void HandlerRequest(std::shared_ptr<Socket> sockfd, InetAddr addr)
    {
        // 短服务
        std::string inbuffer; // 字节流式的队列
        // 不一定能读到完整的报文
        // 我们不做处理，认为我们读到了一个完整的http
        // http如何解决粘包问题的
        ssize_t n = sockfd->Recv(&inbuffer);
        if (n > 0)
        {
            std::string send_str = _cb(inbuffer);
            sockfd->Send(&send_str);
        }
        else if (n == 0)
        {
            LOG(LogLevel::DEBUG) << addr.ToString() << "quit,me too";
        }
        else
        {
            LOG(LogLevel::DEBUG) << addr.ToString() << "read error";
        }

        sockfd->Close();
    }

    void Run()
    {
        while (1)
        {
            signal(SIGCHLD, SIG_IGN);
            // 你要得到一个sockfd,client,addr
            InetAddr addr;
            auto sockfd = _listensocket->Accept(&addr);
            if (sockfd == nullptr)
            {
                continue;
            }
            LOG(LogLevel::DEBUG) << "获取一个新连接：" << addr.ToString();

            // 进程
            if (fork() == 0)
            {
                _listensocket->Close();
                HandlerRequest(sockfd, addr);
                exit(0); // 子进程退出
            }
            sockfd->Close();
        }
    }
    ~TcpServer()
    {
    }

private:
    uint16_t _port;
    std::unique_ptr<Socket> _listensocket;
    callback_t _cb;
};