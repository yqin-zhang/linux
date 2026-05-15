#include "TcpServer.hpp"
#include "Http.hpp"
#include <memory>

// std::string TestHttp(std::string &requeststr)
// {
//     std::cout << "####################" << std::endl;
//     std::cout << requeststr << std::endl;
//     std::cout << "####################" << std::endl;

//     std::string response = "HTTP/1.1 200 OK\r\n";
//     return response;
// }
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " localport" << std::endl;
        exit(0);
    }

    ENABLE_CONSOLE_LOG_STRATEGY();
    // HTTP协议
    std::unique_ptr<Http> http = std::make_unique<Http>();

    // 网络服务
    uint16_t serverport = atoi(argv[1]);
    std::unique_ptr<TcpServer> tsock = std::make_unique<TcpServer>(serverport,
                                                                   [&http](std::string &reqstr) -> std::string
                                                                   {
                                                                       return http->HandlerRequst(reqstr);
                                                                   });
    tsock->Run();

    return 0;
}