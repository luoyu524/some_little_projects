#pragma once
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FORREAD 1
#define FORWRITE 2

const std::string myfifo = "./fifo";
class Fifo
{
public:
    Fifo(const std::string& filename = myfifo) : _filename(filename), _mode(0666), _fd(-1)
    {
    }

    // 创建管道
    void Build()
    {
        // 如果管道文件已存在，就return
        if (IsExist())
            return;

        int n = mkfifo(_filename.c_str(), _mode);
        if (n < 0)
        {
            std::cerr << "mkfifo error: " << strerror(errno) << std::endl;
            exit(1);
        }
        std::cout << "mkfifo success" << std::endl;
    }

    // 打开管道
    void Open(int mode)
    {
        if (mode == FORREAD)
        {
            _fd = open(_filename.c_str(), O_RDONLY);
        }
        else if (mode == FORWRITE)
        {
            _fd = open(_filename.c_str(), O_WRONLY);
        }

        if (_fd < 0)
        {
            std::cerr << "open error: " << strerror(errno) << std::endl;
            exit(2);
        }
        std::cout << "open success" << std::endl;
    }

    // 删除管道
    void Delete()
    {
        if (!IsExist())
        {
            return;
        }
        int n = unlink(_filename.c_str());
        if (n < 0)
        {
            std::cerr << "delete error: " << strerror(errno) << std::endl;
            exit(3);
        }
        std::cout << "delete success" << std::endl;
    }

    // 发送消息
    void Send(std::string& msgin)
    {
        ssize_t n = write(_fd, msgin.c_str(), msgin.size());
    }

    // 接受消息
    int Receive(std::string& msgout)
    {
        char buffer[128];
        ssize_t n = read(_fd, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n] = '\0';
            msgout = buffer;
            return n;
        }
        else if (n == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

private:
    bool IsExist()
    {
        struct stat st;
        // stat函数用于查询一个文件的属性，如果查到了返回0
        // 利用这一点判断管道文件是否存在
        int n = stat(_filename.c_str(), &st);
        if (n == 0)
        {
            return true;
        }
        else
        {
            errno = 0; // 消除这次失败对后面代码的影响
            return false;
        }
    }
    

private:
    std::string _filename;
    mode_t _mode;
    int _fd;
};