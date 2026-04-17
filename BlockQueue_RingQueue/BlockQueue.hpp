#pragma once
#include "Cond.hpp"
#include "Mutex.hpp"
#include <queue>

const int default_cap = 5;

template <class T> class BlockQueue
{
public:
    BlockQueue(int cap = default_cap) : _cap(cap)
    {
    }

    ~BlockQueue()
    {
    }

    void Enqueue(T in)
    {
        // 加锁保护临界区
        LockGuard lg(_mutex);

        // 如果队列为满，此时不能生产，在条件变量下等待
        // wait函数可能会调用失败或系统原因伪唤醒，这里用while判断
        while (_queue.size() == _cap)
        {
            _p_cond.Wait(_mutex);
        }

        // 到这里队列一定有空位置
        _queue.push(in);
        // 已经生产了一个数据，可以唤醒消费者了
        _c_cond.Signal();
    }

    void Pop(T* out)
    {
        // 加锁保护临界区
        LockGuard lg(_mutex);

        // wait函数可能会调用失败或系统原因伪唤醒，这里用while判断
        while (_queue.size() == 0)
        {
            _c_cond.Wait(_mutex);
        }

        // 到这里队列一定有数据
        *out = _queue.front();
        _queue.pop();
        // 取了一个数据，队列一定有一个空位置，可以唤醒生产者了
        _p_cond.Signal();
    }

private:
    std::queue<T> _queue;
    int _cap;
    Mutex _mutex;
    Cond _c_cond;
    Cond _p_cond;
};

// #include <pthread.h>
// #include <queue>

// const int default_cap = 5;

// template <class T> 
// class BlockQueue
// {
// public:
//     BlockQueue(int cap = default_cap) : _cap(cap)
//     {
//         pthread_mutex_init(&_mutex, nullptr);
//         pthread_cond_init(&_c_cond, nullptr);
//         pthread_cond_init(&_p_cond, nullptr);
//     }

//     ~BlockQueue()
//     {
//         pthread_mutex_destroy(&_mutex);
//         pthread_cond_destroy(&_c_cond);
//         pthread_cond_destroy(&_p_cond);
//     }

//     // 放数据，生产者操作
//     void Enqueue(T in)
//     {
//         pthread_mutex_lock(&_mutex);

//         // 如果队列为满，此时不能生产，在条件变量下等待
//         // wait函数可能会调用失败或系统原因伪唤醒，这里用while判断
//         while (_queue.size() == _cap)
//         {
//             pthread_cond_wait(&_p_cond, &_mutex);
//         }

//         // 到这里队列一定有空位置
//         _queue.push(in);
//         // 已经生产了一个数据，可以唤醒消费者了
//         pthread_cond_signal(&_c_cond);

//         pthread_mutex_unlock(&_mutex);
//     }

//     // 消费数据，消费者操作
//     void Pop(T* pop)
//     {
//         pthread_mutex_lock(&_mutex);

//         // 如果队列为空，此时不能消费，在条件变量下等待
//         // wait函数可能会调用失败或系统原因伪唤醒，这里用while判断
//         while (_queue.size() == 0)
//         {
//             pthread_cond_wait(&_c_cond, &_mutex);
//         }

//         // 到这里队列一定有数据
//         *pop = _queue.front();
//         _queue.pop();
//         // 取了一个数据，队列一定有一个空位置，可以唤醒生产者了
//         pthread_cond_signal(&_p_cond);

//         pthread_mutex_unlock(&_mutex);
//     }

// private:
//     std::queue<T> _queue;
//     int _cap;
//     pthread_mutex_t _mutex;
//     pthread_cond_t _c_cond;
//     pthread_cond_t _p_cond;
// };