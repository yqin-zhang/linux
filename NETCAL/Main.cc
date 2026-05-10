#include "TcpServer.hpp"
#include <memory>
int main()
{
    ENABLE_CONSOLE_LOG_STRATEGY();

    std::unique_ptr<TcpServer> tsock = std::make_unique<TcpSocket>(8080);
    tsock->Run();

    while (1)
    {
        sleep(1);
    }
    return 0;
}