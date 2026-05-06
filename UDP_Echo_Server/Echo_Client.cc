#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Logger.hpp"
        
// 客户端必须提前知道服务端的IP和port!!
// ./client_udp server_ip server_port
int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        exit(1);
    }
    
    USE_CONSOLE_LOG_STRATEGY();

    std::string server_ip = argv[1];
    uint16_t server_port = std::stoi(argv[2]);

    // 1. 创建socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        LOG(LogLevel::FATAL) << "create socket error";
        exit(2);
    }

    // 客户端需要有自己的IP和Port信息
    // 但是，客户端不需要显示地bind自己的ip和端口吗！
    // 为什么不让客户端显示bind？bind port可能出现冲突！客户端port只需要具有唯一性即可，具体是几，不重要。
    // 客户端一般会采用随机端口的方式！由OS自主选择端口自动绑定！
    // udp客户端首次发送数据的时候，OS底层会隐式自动帮你进行获取随机端口，然后bind + Port + IP


    // 2. 构建服务端端socket信息 
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port); 
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());

    while(true)
    {
        std::string message;
        // 获取用户输入
        std::cout << "Please Enter# ";
        std::getline(std::cin, message);

        // clinet 发送数据给 server，首次发送即自动bind
        ssize_t n = sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr*)&server, sizeof(server));
        if(n > 0)
        {
            char inbuffer[1024] = {0};
            struct sockaddr_in tmp;
            socklen_t len = sizeof(tmp);
            ssize_t m = recvfrom(sockfd, inbuffer, sizeof(inbuffer)-1, 0, (struct sockaddr*)&tmp, &len);
            if(m > 0)
            {
                inbuffer[m] = 0;
                std::cout << inbuffer << std::endl;
            }
        }
    }


    return 0;
}