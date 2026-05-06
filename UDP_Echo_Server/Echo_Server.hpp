#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Logger.hpp"


const static int default_fd = -1;
const static int default_port = 8888;

enum
{
    SUCCESS = 0,
    USAGE_ERR,
    SOCKET_ERR,
    BIND_ERR,
};

class UdpServer
{
public:

    // 服务端端口号，必须自己设定好。客户端必须提前知道服务器端口号，才能访问
    UdpServer(uint16_t port = default_port)
        : _port(port),
          _sockfd(default_fd)
    {}

    ~UdpServer()
    {
        close(_sockfd);
    }

    void Init()
    {
        // 第一步: 创建socket
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0); // IPv4, UDP通信
        if (_sockfd < 0)
        {
            LOG(LogLevel::FATAL) << "create socket error";
            exit(SOCKET_ERR);
        }
        LOG(LogLevel::INFO) << "create socket success, sockfd: " << _sockfd;

        // 第二步: 填充IP和端口号信息
        struct sockaddr_in local;
        bzero(&local, sizeof(local)); // 清空local
        local.sin_family = AF_INET; // IPv4
        local.sin_port = htons(_port); // 写入端口号
        local.sin_addr.s_addr = INADDR_ANY; // 当前机器上任意IP地址都计入

        // 第三步：bind socket信息
        int n = bind(_sockfd, (struct sockaddr *)&local, sizeof(local)); // 强转类型
        if (n < 0)
        {
            LOG(LogLevel::FATAL) << "bind socket error";
            exit(BIND_ERR);
        }
        LOG(LogLevel::INFO) << "bind socket success"<< ", port: " << _port;
    }

    void Start()
    {
        char inbuffer[1024];
        while (true)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);

            // 读取数据
            ssize_t n = recvfrom(_sockfd, inbuffer, sizeof(inbuffer)-1, 0, (struct sockaddr *)&peer, &len);
            if (n > 0)
            {
                // 网络序列转为主机序列
                uint16_t client_port = ntohs(peer.sin_port);

                // 将sin_addr形式的ip地址，用inet_ntoa函数，转为xx.xx.xx.xx风格字符串，序列也会自动转换
                std::string client_ip = inet_ntoa(peer.sin_addr); 

                std::string client_address = "[" + client_ip + ":" + std::to_string(client_port) + "]# ";

                // 客户端发来的数据
                inbuffer[n] = 0;
                LOG(LogLevel::INFO) << client_address << inbuffer;
                
                std::string echo_string = "server echo# ";
                echo_string += inbuffer;

                // 向客户端回显数据, peer中记录了客户端是谁
                sendto(_sockfd, echo_string.c_str(), echo_string.size(), 0, (struct sockaddr *)&peer, len);
            }
            else
            {
                LOG(LogLevel::ERROR) << "recvfrom error";
            }
        }
    }

private:
    int _sockfd;
    uint16_t _port;  // 服务端自己设置好，port必须是固定的！因为一定是客户端发起第一次通信，客户端必须提前知道服务端是谁
};