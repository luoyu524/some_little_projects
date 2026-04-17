// Sem.hpp
#pragma once
#include <semaphore.h>
class Sem
{
public:
    Sem(int init_val) // 传递信号量初始值
    {
        if (init_val >= 0)
        {
            sem_init(&_sem, 0, init_val);
        }
    }
    ~Sem()
    {
        sem_destroy(&_sem);
    }
    void P()
    {
        sem_wait(&_sem);
    }
    void V()
    {
        sem_post(&_sem);
    }

private:
    sem_t _sem;
};