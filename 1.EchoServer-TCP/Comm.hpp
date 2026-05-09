#ifndef __COMM_HPP__
#define __COMM_HPP__

enum
{
    OK,
    SOCK_CREATE_ERR, // 套接字文件打开失败
    SOCK_BIND_ERR,   // 绑定网络通讯失败
    SOCK_LISTEN_ERR, // 监听套接字失败
    SOCK_CONNECT_ERR,
    FORK_ERR
};

#endif