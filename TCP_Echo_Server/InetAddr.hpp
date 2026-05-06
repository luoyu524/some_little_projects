// InetAddr.hpp 对网络地址进程描述
#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <strings.h>
#include <sys/socket.h>

class InetAddr
{
public:
    InetAddr() = default;

    InetAddr(const struct sockaddr_in& address) : _address(address), _len(sizeof(address))
    {
        _ip = inet_ntoa(_address.sin_addr);
        _port = ntohs(_address.sin_port);
    }

    InetAddr(uint16_t port, const std::string& ip) : _ip(ip), _port(port)
    {
        // 自动构造struct sockaddr_in 类型网络地址
        bzero(&_address, sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_port = htons(_port);
        _address.sin_addr.s_addr = inet_addr(_ip.c_str());
        _len = sizeof(_address);
    }

    InetAddr(uint16_t port) : _port(port)
    {
        bzero(&_address, sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_port = htons(_port);
        _address.sin_addr.s_addr = INADDR_ANY;
        _len = sizeof(_address);
    }

    bool operator==(const InetAddr& addr)
    {
        return (this->_ip == addr._ip) && (this->_port == addr._port);
    }

    std::string ToString()
    {
        return "[" + _ip + ":" + std::to_string(_port) + "]";
    }

    struct sockaddr* GetNetAddress()
    {
        return (struct sockaddr*)&_address;
    }

    socklen_t Len()
    {
        return _len;
    }

    ~InetAddr()
    {
    }

private:
    struct sockaddr_in _address;
    socklen_t _len;
    std::string _ip;
    uint16_t _port;
};