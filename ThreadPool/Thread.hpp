#ifndef _THREAD_
#define _THREAD_

#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <functional>

static int gnumber = 1;
using callback_t = std::function<void()>;

class Thread
{
private:
    static void* Thread_Routine(void* args)
    {
        Thread* self = static_cast<Thread*>(args);
        pthread_setname_np(self->_tid, self->_name.c_str());
        self->_task(); // 执行任务
        return nullptr;
    }

public:
    Thread(callback_t task) : _task(task), _tid(-1), _joinable(true), _result(nullptr)
    {
        _name = "NewThread-" + std::to_string(gnumber++);
    }

    void Start()
    {
        pthread_create(&_tid, nullptr, Thread_Routine, this);
    }

    void Join()
    {
        if (_joinable)
        {
            pthread_join(_tid, &_result);
            std::cout << "线程已回收" << std::endl;
        }
        else
        {
            std::cerr << "线程不可被等待" << std::endl;
        }
    }

    void Detach()
    {
        if(_joinable)
        {
            pthread_detach(_tid);
            _joinable = false;
        }
        else
        {
            std::cerr<<"线程已被分离" << std::endl;
        }
    }

    ~Thread()
    {
    }

private:
    std::string _name;
    pthread_t _tid;
    callback_t _task;
    void* _result;
    bool _joinable;
};

#endif