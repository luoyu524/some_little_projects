#pragma once
#include "InetAddr.hpp"
#include "Logger.hpp"
#include "ThreadPool.hpp"
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using task_t = std::function<void()>;

enum
{
    SUCCESS = 0,
    USAGE_ERR,
    SOCKET_ERR,
    BIND_ERR,
    LISTEN_ERR,
    FORK_ERR
};

static const int gbacklog = 16;
static const uint16_t gport = 8888;

class TcpServer
{
public:
    TcpServer(uint16_t port = gport) : _port(port)
    {
    }

    void InitServer()
    {
        // 1. 创建socket
        _listensockfd = socket(AF_INET, SOCK_STREAM, 0); // TCP, 使用SOCK_STREAM选项
        if (_listensockfd < 0)
        {
            LOG(LogLevel::FATAL) << "create socket error";
            exit(SOCKET_ERR);
        }
        LOG(LogLevel::INFO) << "create socket success: " << _listensockfd;

        // 2. 填充本地socket信息
        InetAddr local(_port); // 任意地址bind

        // 3. bind
        int n = bind(_listensockfd, local.GetNetAddress(), local.Len());
        if (n < 0)
        {
            LOG(LogLevel::FATAL) << "bind error";
            exit(BIND_ERR);
        }
        LOG(LogLevel::INFO) << "bind socket success";

        // 4. TCP是面向连接的，TCP服务器必须先要处于listen状态。
        n = listen(_listensockfd, gbacklog);
        if (n < 0)
        {
            LOG(LogLevel::FATAL) << "listen error";
            exit(LISTEN_ERR);
        }
        LOG(LogLevel::INFO) << "listen socket success";
    }

    void Start()
    {
        while (1)
        {
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);
            // 5. 获取连接
            int sockfd = accept(_listensockfd, (struct sockaddr*)&clientaddr, &len);
            if (sockfd < 0)
            {
                LOG(LogLevel::WARNING) << "accept errr!";
                continue;
            }
            LOG(LogLevel::INFO) << "accept success, sockfd: " << sockfd;

            // 处理通信任务：多进程、多线程、线程池，这里使用线程池
            // 将通信任务放入线程池
            InetAddr clientaddress(clientaddr);
            ThreadPool<task_t>::Instance()->Enqueue([this, sockfd, clientaddress]() -> void
                                                    { this->serviceIO(sockfd, clientaddress); });
        }
    }

    void serviceIO(int sockfd, InetAddr address)
    {
        // 长连接，长服务
        while (1)
        {
            char inbuffer[1024] = {0};
            // 读消息
            ssize_t n = read(sockfd, inbuffer, sizeof(inbuffer) - 1);
            if (n > 0)
            {
                inbuffer[n] = 0;
                LOG(LogLevel::INFO) << address.ToString() << " say# " << inbuffer;

                // 写回消息
                std::string echo_string = "server echo# ";
                echo_string += inbuffer;
                write(sockfd, echo_string.c_str(), echo_string.size());
            }
            else if (n == 0)
            {
                LOG(LogLevel::INFO) << "client quit, address: " << address.ToString();
                break;
            }
            else
            {
                LOG(LogLevel::ERROR) << "client read error, address: " << address.ToString();
                break;
            }
        }
        close(sockfd);
    }

    ~TcpServer()
    {
        close(_listensockfd);
    }

private:
    uint16_t _port;
    // 不需要显示包含ip
    int _listensockfd; // 专门用于监听的套接字
};