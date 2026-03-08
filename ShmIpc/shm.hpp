#pragma once
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define PATHNAME "/usr/bin"
#define PROJ_ID 0x66

class Shm
{
public:
    Shm() : _shmid(-1), _size(4096), _start_add(nullptr)
    {
    }

    // 服务端负责创建共享内存
    void Creat()
    {
        GetHelper(IPC_CREAT | IPC_EXCL | 0666);
    }

    // 客户端要找到共享内存
    void Get()
    {
        GetHelper(IPC_CREAT);
    }

    // 挂接
    void Attach()
    {
        _start_add = shmat(_shmid, nullptr, 0);
        if ((long long)_start_add == -1)
        {
            perror("shmat");
            exit(2);
        }
    }

    // 使用共享内存需要得到地址
    void* GetAdd()
    {
        return _start_add;
    }

    int size()
    {
        return _size;
    }

    // 去关联
    void Dettch()
    {
        int n = shmdt(_start_add);
        if (n < 0)
        {
            perror("shmdt");
            exit(3);
        }
    }

    // 删除
    void Delete()
    {
        int n = shmctl(_shmid, IPC_RMID, nullptr);
        if (n < 0)
        {
            perror("shmctl");
            exit(4);
        }
    }

private:
    void GetHelper(int shmflag)
    {
        key_t key = ftok(PATHNAME, PROJ_ID);
        _shmid = shmget(key, _size, shmflag);

        if (_shmid < 0)
        {
            perror("shmget");
            exit(1);
        }
    }

private:
    int _shmid;
    int _size;
    void* _start_add;
};