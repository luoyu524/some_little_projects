#pragma once
#include "Cond.hpp"
#include "Logger.hpp"
#include "Mutex.hpp"
#include "Thread.hpp"
#include <pthread.h>
#include <queue>
#include <unistd.h>
#include <vector>

const int defaultnum = 5;

template <class T> 
class ThreadPool
{
private:
    void HandlerTask()
    {
        char name[128];
        pthread_getname_np(pthread_self(), name, sizeof name);
        while (1)
        {
            T task;
            {
                LockGuard lg(_mutex);
                // 如果线程池还在运行，但是没有任务，就阻塞等待任务
                while (_tasks.empty() && _isRunning)
                {
                    _cond.Wait(_mutex);
                }
                // 如果线程池不运行，且没有任务了，就退出
                if (!_isRunning && _tasks.empty())
                {
                    _mutex.Unlock();
                    break;
                }

                task = _tasks.front();
                _tasks.pop();
            }
            task();
            LOG(LogLevel::INFO) << name << "处理任务: " << task.Result();
            sleep(1);
        }
    }

public:
    ThreadPool(int thread_num = defaultnum) : _thread_num(thread_num), _isRunning(false)
    {
        for (int i = 0; i < _thread_num; i++)
        {
            _threads.emplace_back([this]() { this->HandlerTask(); });
        }
    }

    void Start()
    {
        _isRunning = true;
        for (auto& thread : _threads)
        {
            thread.Start();
        }
    }

    void Stop()
    {
        LockGuard lg(_mutex);
        _isRunning = false;
        _cond.BroadCast();
    }

    void Wait()
    {
        for (auto& thread : _threads)
        {
            thread.Join();
        }
    }

    void Enqueue(T in)
    {
        LockGuard lg(_mutex);
        _tasks.push(in);
        _cond.Signal();
    }

public:
    // 单例模式，全局只有一个线程池对象
    static ThreadPool<T>* Instance()
    {
        if (_instance == nullptr)
        {
            LockGuard lg(_lock);
            if (_instance == nullptr)
            {
                _instance = new ThreadPool<T>();
                _instance->Start();
                LOG(LogLevel::INFO) << "第一次使用线程池，创建线程池对象";
            }
        }
        return _instance;
    }

private:
    static ThreadPool<T>* _instance;
    static Mutex _lock;

private:
    int _thread_num;
    std::vector<Thread> _threads;
    std::queue<T> _tasks; // 任务队列,临界资源
    bool _isRunning;
    Mutex _mutex;
    Cond _cond;
};

template <class T> 
ThreadPool<T>* ThreadPool<T>::_instance = nullptr;
template <class T> 
Mutex ThreadPool<T>::_lock;
