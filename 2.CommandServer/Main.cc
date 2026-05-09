#include "CommandServer.hpp"
#include "Command.hpp"
#include <memory>

void CommandExec(std::string proc)
{
    // 线程内部可以创建进程吗？可以！
    // 1.创建管道
    // 2.fork()
    // 3.命令分析exec，1->pipe[1]
}
int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " localport" << std::endl;
        exit(0);
    }

    uint16_t serverport = std::stoi(argv[1]);

    ENABLE_CONSOLE_LOG_STRATEGY();
    Command cmdobj;
    // 回调
    std::unique_ptr<CommandServer> tsvr = std::make_unique<CommandServer>(
        [&cmdobj](const std::string &cmd) -> std::string
        {
            return cmdobj.Exec(cmd);
        },
        serverport);

    tsvr->Init();
    tsvr->Start();
    return 0;
}