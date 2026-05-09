#include "TcpEchoServer.hpp"
#include <memory>

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " localport" << std::endl;
        exit(0);
    }

    uint16_t serverport = std::stoi(argv[1]);

    ENABLE_CONSOLE_LOG_STRATEGY();
    std::unique_ptr<TcpEchoServer> tsvr = std::make_unique<TcpEchoServer>();

    tsvr->Init();
    tsvr->Start();
    return 0;
}