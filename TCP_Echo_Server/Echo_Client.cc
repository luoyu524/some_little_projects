#include "InetAddr.hpp"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "./client_tcp server_ip server_port" << std::endl;
        exit(1);
    }

    std::string server_ip = argv[1];
    uint16_t server_port = std::stoi(argv[2]);

    // 1. 创建tcp套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "socket error" << std::endl;
        exit(2);
    }

    // 客户端不需要显示bind

    // 2. 向服务端发起连接
    InetAddr serveraddress(server_port, server_ip);
    int n = connect(sockfd, (struct sockaddr*)serveraddress.GetNetAddress(), serveraddress.Len());
    if (n < 0)
    {
        std::cerr << "connect to " << serveraddress.ToString() << " failed!" << std::endl;
        exit(3);
    }
    std::cerr << "connect to " << serveraddress.ToString() << " success!" << std::endl;

    // 3. 通信
    while (1)
    {
        std::string line;
        std::cout << "please Enter# ";
        std::getline(std::cin, line);

        write(sockfd, line.c_str(), line.size());

        char inbuffer[1024];
        ssize_t n = read(sockfd, inbuffer, sizeof(inbuffer));
        if (n > 0)
        {
            inbuffer[n] = 0;
            std::cout << inbuffer << std::endl;
        }
        else if (n == 0)
        {
            std::cout << "read end of file!" << std::endl;
            break;
        }
        else
        {
            std::cerr << "read error!" << std::endl;
            break;
        }
    }

    return 0;
}