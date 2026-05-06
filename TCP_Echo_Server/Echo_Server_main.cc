#include "Echo_Server.hpp"
#include "Logger.hpp"
#include <memory>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "./server_tcp port" << std::endl;
        exit(USAGE_ERR);
    }
    USE_CONSOLE_LOG_STRATEGY();
    uint16_t server_port = std::stoi(argv[1]);

    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(server_port);
    tsvr->InitServer();
    tsvr->Start();

    return 0;
}