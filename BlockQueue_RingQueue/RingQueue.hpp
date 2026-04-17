// RingQueue.hpp
#pragma once
#include "Mutex.hpp"
#include "Sem.hpp"
#include <vector>

const int default_cap = 5;

template <class T> class RingQueue
{
public:
    RingQueue(int cap = default_cap) 
        : _cap(cap),
          _rq(cap),
          _c_step(0),
          _p_step(0),
          _data(0),
          _blank(cap)
    {}
    ~RingQueue()
    {}

    void Enqueue(T& in)
    {
        // 生产者放数据，需要申请一个空位置
        _blank.P();

        {
            LockGuard lg(_p_mutex);
            _rq[_p_step] = in;
            _p_step++;
            _p_step %= _cap;
        }

        // 此时多了一个数据，_data信号量可以释放
        _data.V();
    }

    void Pop(T* out)
    {
        // 消费者取数据，需要申请一个数据
        _data.P();

        {
            LockGuard lg(_c_mutex);
            *out = _rq[_c_step];
            _c_step++;
            _c_step %= _cap;
        }

        // 此时多了一个空位置，_blank信号量可以释放
        _blank.V();
    }

private:
    std::vector<T> _rq;
    int _cap;    // 最大容量
    int _c_step; // 消费者取数据位置
    int _p_step; // 生产者放数据位置

    Sem _data;   // 消费者需要的数据资源, 初始为0
    Sem _blank;  // 生产者需要的空格资源, 初始为cap

    Mutex _c_mutex; // 维护消费者之间的互斥
    Mutex _p_mutex; // 维护生产者之间的互斥
};