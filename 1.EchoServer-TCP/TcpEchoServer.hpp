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
#include "Thread.hpp"
#include "ThreadPool.hpp"

using namespace LogModule;

static const int gdefaultfd = -1;
static const int gbacklog = 8;
static const int gport = 8080;

using task_t = std::function<void()>;
class TcpEchoServer
{
private:
    // 长任务->sockfd->长连接
    // 短任务->短连接
    void HandlerIO(int sockfd, InetAddr client)
    {
        char buffer[1024];
        while (1)
        {

            // 预留一个位置给字符串结束符 \0。
            // 后果：数组越界，可能导致程序崩溃或数据损坏。
            // 在网络中，Read返回值为0，代表客户端关闭
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                std::string echo_string = "server echo#";
                echo_string += buffer;
                write(sockfd, echo_string.c_str(), echo_string.size());
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
    TcpEchoServer(uint16_t port = gport)
        : _listensockfd(gdefaultfd), _port(port)
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
        ThreadData(int sockfd, TcpEchoServer *self, const InetAddr addr)
            : _sockfd(sockfd),
              _self(self),
              _addr(addr)
        {
        }

    public:
        int _sockfd;
        TcpEchoServer *_self;
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

            // 多进程，多线程
            // 1.效率问题，创建进程线程
            // 2.执行流个数没有上限

            // 进程池4：
            ThreadPool<task_t>::GetInstance()->Enqueue([this, sockfd, clientaddr]()
                                                       { this->HandlerIO(sockfd, clientaddr); });

            // //version3 多线程做法
            // pthread_t tid;
            // //多线程是如何看待主线程曾经打开的文件描述符表！共享
            // //主线程和新线程需要关闭历史fd吗？不需要
            // ThreadData *td=new ThreadData(sockfd,this,clientaddr);
            //    //sockfd字面值拷贝给了void*指针，再强转成int*，，就能拿到sockfd
            // pthread_create(&tid,nullptr,Routine,(void*)sockfd);

            // handler sockfd
            // version1-单进程的
            // HandlerIO(sockfd,clientaddr);

            // version2---多进程
            // 创建子进程，子进程是如何看待父进程的fd的？
            // sockfd可以被子进程继承

            // pid_t id = fork();
            // if (id < 0)
            // {
            //     LOG(LogLevel::FATAL) << "资源不够，创建子进程失败";
            //     exit(FORK_ERR);
            // }
            // else if (id == 0)
            // {
            //     // 子进程--孙子进程创建成功后退出
            //     //child -> sockfd -> 也可以看见_listensockfd
            //     // 一种防御编程--关闭监听套接字（孙子进程不需要）
            //     close(_listensockfd);
            //     if (fork() > 0)
            //     {
            //         exit(OK);
            //     }

            //     // 孙子进程--孤儿进程--系统领养
            //     //负责客户端通信，数据接收
            //     HandlerIO(sockfd, clientaddr);
            //     exit(OK);
            // }
            // else
            // {
            //     //1.关闭无用fd 2.规避fd泄露
            //     close(sockfd);  // 父进程不需要通信套接字

            //     // 父进程--只关注子进程
            //     //等待子进程后，返回继续连接
            //     //father -> _listensockfd -> sockfd
            //     pid_t rid = waitpid(id, nullptr, 0);
            //     (void)rid;
            // }
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