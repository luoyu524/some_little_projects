// Socket.hpp
#pragma once
#include "InetAddr.hpp"
#include "Logger.hpp"
#include <memory>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

const int gbacklog = 16;

// 设计模式, 模版方法模式
class Socket
{
public:
    ~Socket() {}

protected:
    virtual void CreateSocket() = 0;
    virtual void BindSocket(uint16_t port) = 0;
    virtual void ListenSocket() = 0;

public:
    // Tcp服务端创建套接字方法
    void BuildTcpServerSock(uint16_t port)
    {
        CreateSocket();
        BindSocket(port);
        ListenSocket();
    }

    // Tcp客户端创建套接字方法
    void BuildTcpClientSock()
    {
        CreateSocket();
    }

    virtual std::shared_ptr<Socket> Accept(InetAddr* clientaddr) = 0;
    virtual int Sockfd() = 0;
    virtual int Recv(std::string* out) = 0;
    virtual int Send(const std::string& in) = 0;
    virtual void Close() = 0;
    virtual bool Connect(InetAddr& addr) = 0;

    // 今天我们只使用TCP实现，UDP不考虑了
    // void BuildUdpServerSock(uint16_t port)
    // {
    //     CreateSocket();
    //     BindSocket(port);
    // }
};

// class UdpSocket : public Socket
// {...}

class TcpSocket : public Socket
{
public:
    TcpSocket(int sockfd = -1) : _sockfd(sockfd) {}

    ~TcpSocket() {}

    void CreateSocket() override
    {
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0)
        {
            LOG(LogLevel::FATAL) << "creat socket error";
            exit(1);
        }
    }

    void BindSocket(uint16_t port) override
    {
        InetAddr addr(port);
        if (bind(_sockfd, addr.GetNetAddress(), addr.Len()) != 0)
        {
            LOG(LogLevel::FATAL) << "bind socket error";
            exit(1);
        }
    }

    void ListenSocket() override
    {
        if (listen(_sockfd, gbacklog) != 0)
        {
            LOG(LogLevel::FATAL) << "listen socket error";
            exit(1);
        }
    }

    std::shared_ptr<Socket> Accept(InetAddr* clientaddr) override
    {
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        int sockfd = accept(_sockfd, (struct sockaddr*)&addr, &len);
        if (sockfd < 0)
        {
            LOG(LogLevel::WARNING) << "accpet error";
            return nullptr;
        }
        *clientaddr = addr;
        return std::make_shared<TcpSocket>(sockfd);
    }

    int Sockfd() override
    {
        return _sockfd;
    }

    int Recv(std::string* out) override
    {
        char inbuffer[1024];
        ssize_t n = recv(_sockfd, inbuffer, sizeof(inbuffer) - 1, 0);
        if (n > 0)
        {
            inbuffer[n] = 0;
            *out += inbuffer;
        }
        return n;
    }

    int Send(const std::string& in) override
    {
        return send(_sockfd, in.c_str(), in.size(), 0);
    }

    void Close() override
    {
        if (_sockfd >= 0)
        {
            close(_sockfd);
            _sockfd = -1;
        }
    }

    bool Connect(InetAddr& addr) override
    {
        int n = connect(_sockfd, addr.GetNetAddress(), addr.Len());
        if (n < 0)
        {
            return false;
        }
        return true;
    }

private:
    int _sockfd;
};
