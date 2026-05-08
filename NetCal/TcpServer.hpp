// TcpServer.hpp
#pragma once
#include "InetAddr.hpp"
#include "Logger.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"
#include <functional>
#include <memory>
#include <string>

using task_t = std::function<void()>;

static const uint16_t gport = 8888;
using Hander_t = std::function<std::string(std::string&)>;

class TcpServer
{
public:
    TcpServer(Hander_t handler, uint16_t port = gport)
        : _port(port), _handler(handler), _listensock(std::make_unique<TcpSocket>())
    {
        _listensock->BuildTcpServerSock(port);
        LOG(LogLevel::INFO) << "create listen socket success: " << _listensock->Sockfd();
    }

    void Start()
    {
        while (1)
        {
            InetAddr clientaddr;
            auto sockfd = _listensock->Accept(&clientaddr);
            if (!sockfd)
            {
                continue;
            }
            LOG(LogLevel::INFO) << "accept client success, sockfd: " << sockfd->Sockfd();

            // 处理通信任务可以多进程、多线程、线程池，这里使用线程池
            // 将通信任务放入线程池
            ThreadPool<task_t>::Instance()->Enqueue([this, sockfd, clientaddr]() -> void
                                                    { this->serviceIO(sockfd, clientaddr); });
        }
    }
    
    ~TcpServer()
    {
        _listensock->Close();
    }

private:
    void serviceIO(std::shared_ptr<Socket> sockfd, const InetAddr& clientaddr)
    {
        std::string inbuffer, outbuffer;
        while (1)
        {
            int n = sockfd->Recv(&inbuffer);
            if (n <= 0)
            {
                LOG(LogLevel::WARNING) << "recv, client quit: " << clientaddr.ToString();
                break;
            }
            LOG(LogLevel::INFO) << "inbuffer:\n" << inbuffer;

            // 处理获取的数据流，返回需要发回的数据流
            outbuffer += _handler(inbuffer);
            if (outbuffer.empty())
            {
                continue;
            }
            LOG(LogLevel::INFO) << "outbuffer:\n" << outbuffer;

            n = sockfd->Send(outbuffer);
            // 每次发送完的内容清空
            outbuffer.clear();
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "send, client quit: " << clientaddr.ToString();
                break;
            }
        }
        sockfd->Close();
    }

private:
    uint16_t _port;
    std::unique_ptr<Socket> _listensock;
    Hander_t _handler;
};
