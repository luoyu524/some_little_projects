// NetCalClient.cc
#include "InetAddr.hpp"
#include "Protocol.hpp"
#include "Socket.hpp"
#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Client.exe ip port" << std::endl;
        return 1;
    }
    std::string server_ip = argv[1];
    uint16_t server_port = std::stoi(argv[2]);
    InetAddr serveraddr(server_port, server_ip);

    std::unique_ptr<Socket> socket = std::make_unique<TcpSocket>();
    socket->BuildTcpClientSock();
    bool n = socket->Connect(serveraddr);
    if (!n)
    {
        std::cerr << "connect error: " << serveraddr.ToString() << std::endl;
        return 2;
    }

    Protocol protocol([](Response& resp){ 
        std::cout << "result: " << resp.Getres() << '[' << resp.Getcode() << ']' << std::endl; 
    });

    std::string inbuffer;
    while(1)
    {
        // 攒三次数据再发送
        std::string outbuffer;
        for(int i = 0; i < 3; i++)
        {
            int x, y;
            char oper;
            std::cout << "输入x:" << std::endl;
            std::cin >> x;
            std::cout << "输入y:" << std::endl;
            std::cin >> y;
            std::cout << "输入oper:" << std::endl;
            std::cin >> oper;

            // 定义请求对象
            Request req(x, y, oper);

            // 序列化
            std::string req_json;
            req.Serialize(&req_json);

            // 添加报头
            std::string send_req = protocol.Packet(req_json);
            outbuffer += send_req;
        }
        std::cout << '\n' << outbuffer << std::endl;

        // 发送请求
        socket->Send(outbuffer);
        // 接收应答
        socket->Recv(&inbuffer);
        // 解析应答
        protocol.ParseResponse(inbuffer);
    }
    return 0;
}
 