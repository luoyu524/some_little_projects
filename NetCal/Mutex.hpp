#pragma once
#include<pthread.h>
class Mutex
{
public:
    Mutex()
    {
        pthread_mutex_init(&_lock, nullptr);
    }
    ~Mutex()
    {
        pthread_mutex_destroy(&_lock);
    }
    void Lock()
    {
        pthread_mutex_lock(&_lock);
    }
    void Unlock()
    {
        pthread_mutex_unlock(&_lock);
    }
    pthread_mutex_t* Ptr()
    {
        return &_lock;
    }
private:
    pthread_mutex_t _lock;
};

// RAII风格用法
class LockGuard
{
public:
    LockGuard(Mutex& lock)
        :_lockref(lock)
    {
        _lockref.Lock();
    }
    ~LockGuard()
    {
        _lockref.Unlock();
    }
private:
    Mutex& _lockref;
};

/*
后续使用我们自己封装的锁时，可以写成：
{
    LockGuard lock(mutex);
    // ...
    // 临界区
}
{}划定作用域, 利用LockGuard自动调用构造析构完成加锁和解锁
*/
