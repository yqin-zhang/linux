#ifndef __TCP_ECHO_SERVER_HPP__
#define __TCP_ECHO_SERVER_HPP__

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> //sockaddr_in
#include <cstdlib>      //exit
#include <sys/wait.h>
#include <pthread.h>
#include "Comm.hpp"
#include "Logger.hpp"
#include "InetAddr.hpp"
#include <functional>

using namespace LogModule;

static const int gdefaultfd = -1;
static const int gbacklog = 8;
static const int gport = 8080;
// 回调函数，解耦，业务逻辑更灵活
using callback_t = std::function<std::string(const std::string &)>;

// 只负责IO通信
class CommandServer
{
private:
    // 长任务->sockfd->长连接
    // 短任务->短连接
    void HandlerIO(int sockfd, InetAddr client)
    {
        char buffer[1024];
        while (1)
        {
            buffer[0] = 0;
            // 约定：你给我发过来的命令字符串！ls -a -l
            // 面向字节流
            //"ls -a -l"->"ls -"->"a -l"->为什么会这样？UDP不存在这样的问题
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                LOG(LogLevel::DEBUG) << client.ToString() << "say: " << buffer;
                std::string result = _cb(buffer);

                write(sockfd, result.c_str(), result.size());
            }
            else if (n == 0)
            {
                LOG(LogLevel::INFO) << "client" << client.ToString() << "quit";
                break;
            }
            else
            {
                LOG(LogLevel::WARNING) << "read client" << client.ToString() << "error,sockfd: " << sockfd;
                break;
            }
        }
        close(sockfd); // 为什么一定要关闭
    }

public:
    // 失败返回的-1，默认为-1
    CommandServer(callback_t cb, uint16_t port = gport)
        : _listensockfd(gdefaultfd), _port(port), _cb(cb)
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

        InetAddr local(_port);

        // 因为成功返回0，失败返回-1
        if (bind(_listensockfd, local.Addr(), local.length()) != 0) // 不成功
        {
            // 打印日志
            LOG(LogLevel::FATAL) << "bind socket error";
            // 退出码
            exit(SOCK_BIND_ERR);
        }
        LOG(LogLevel::INFO) << "bind tcp socket success" << _listensockfd;

        // 3.set socket listen
        // 一个TCP server listen ,启动之后，服务器已经算运行了
        if (listen(_listensockfd, gbacklog) != 0)
        {
            LOG(LogLevel::FATAL) << "listen socket error";
            // 退出码
            exit(SOCK_LISTEN_ERR);
        }
        LOG(LogLevel::INFO) << "listen socket success" << _listensockfd;
    }
    static void *Routine(void *args)
    {
        ThreadData *td = static_cast<ThreadData *>(args);
        pthread_detach(pthread_self());
        td->_self->HandlerIO(td->_sockfd, td->_addr);
        delete td; // 记得释放内存
        return nullptr;
    }
    class ThreadData
    {
    public:
        ThreadData(int sockfd, CommandServer *self, const InetAddr addr)
            : _sockfd(sockfd),
              _self(self),
              _addr(addr)
        {
        }

    public:
        int _sockfd;
        CommandServer *_self;
        InetAddr _addr;
    };

    void Start()
    {
        // signal(SIGCHLD,SIG_IGN);//最佳实践

        // 死循环
        while (1)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            // sockfd是通信套接字
            int sockfd = accept(_listensockfd, (struct sockaddr *)&peer, &len);

            if (sockfd < 0)
            {
                LOG(LogLevel::WARNING) << "accept client error";
                continue;
            }
            InetAddr clientaddr(peer);
            LOG(LogLevel::INFO) << "获取新连接成功,sockfd is:" << sockfd
                                << "client addr: " << clientaddr.ToString();

            // version3 多线程做法
            pthread_t tid;
            // 多线程是如何看待主线程曾经打开的文件描述符表！共享
            // 主线程和新线程需要关闭历史fd吗？不需要
            ThreadData *td = new ThreadData(sockfd, this, clientaddr);
            // sockfd字面值拷贝给了void*指针，再强转成int*，，就能拿到sockfd
            pthread_create(&tid, nullptr, Routine, (void *)sockfd);
        }
    }
    ~CommandServer()
    {
    }

private:
    int _listensockfd; // 监听套接字

    uint16_t _port; // 2字节
    callback_t _cb;
};

#endif