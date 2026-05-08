#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void Usage(const std::string& proc)
{
    std::cout << "Usage: " << proc << " serverip serverport" << std::endl;
    std::cout << "Example: " << proc << " 127.0.0.1 8080" << std::endl;
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        Usage(argv[0]);
        exit(0);
    }
    
    std::string serverip = argv[1];  // 如果需要指定IP
    uint16_t serverport = std::stoi(argv[2]); // 字符串转整数
    
    // 创建 socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        std::cout << "create socket error" << std::endl;
        return 0;
    }
    
    // client要不要，显示的bind自己的ip和端口？不要
    // client要不要 隐式bind IP和端口？要
    // 为什么？client会在自己OS的帮助下，随机bind端口号
    
    // 配置服务器地址
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(serverport);
    // 如果服务端在本机，用 127.0.0.1；如果是远程，用对应的 IP
    inet_pton(AF_INET, serverip.c_str(), &server.sin_addr);
    // server.sin_addr.s_addr = INADDR_ANY;  // 这样写不推荐，会连接不到服务器
    
    socklen_t server_len = sizeof(server);
    
    while(1)
    {
        std::string line;
        std::cout << "请输入消息: ";
        std::getline(std::cin, line);
        
        if(line == "quit" || line == "exit")
        {
            break;
        }
        
        // 写：发送消息给服务端
        sendto(sockfd, line.c_str(), line.size(), 0,
               (struct sockaddr*)&server, server_len);
        
        // 读：接收服务端回声
        struct sockaddr_in temp;
        socklen_t len = sizeof(temp);
        char buffer[1024];
        int m = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr*)&temp, &len);
        
        if(m > 0)
        {
            buffer[m] = '\0';
            std::cout << "服务端回声: " << buffer << std::endl;
        }
    }
    
    close(sockfd);
    return 0;
}