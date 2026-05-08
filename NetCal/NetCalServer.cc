// NetCalServer.cc
#include "Calculator.hpp"
#include "Protocol.hpp"
#include "TcpServer.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "./Server.exe port" << std::endl;
        return 1;
    }
    uint16_t port = std::stoi(argv[1]);

    // 定义计算器对象
    std::unique_ptr<Calculator> cal = std::make_unique<Calculator>();

    // 定义协议对象
    std::unique_ptr<Protocol> protocol = std::make_unique<Protocol>(
        [&cal](Request& req) -> Response 
        { 
            return cal->Execute(req); 
        }
    );

    // 定义服务器对象
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(
        [&protocol](std::string& inbuffer)->std::string
        {
            return protocol->ParseRequest(inbuffer);
        },
        port
    );

    // 启动
    tsvr->Start();


    return 0;
}
