#include"Echo_Server.hpp"
#include "Logger.hpp"
#include <memory>

// 执行程序命令行参数为 ./server_udp port
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        exit(USAGE_ERR);
    }

    USE_CONSOLE_LOG_STRATEGY();

    uint16_t server_port = std::stoi(argv[1]);

    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>(server_port);

    usvr->Init();
    usvr->Start();

    return 0;
}