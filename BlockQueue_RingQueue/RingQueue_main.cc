#include "RingQueue.hpp"
#include "Thread.hpp"
#include <iostream>
#include <unistd.h>

int num = 1;
Mutex lock;
Mutex lock1;
RingQueue<int> rq;

void ProductorRoutine()
{
    while(1)
    {
        char name[16];
        pthread_getname_np(pthread_self(), name, sizeof name);
        int in;
        {
            LockGuard lg(lock);
            in = num;
            num++;
        }
        rq.Enqueue(in);
        {
            LockGuard lg1(lock1);
        std::cout << "线程" << name << " 生产数据" << in << std::endl;
        }
        sleep(1);
    }
}

void ConsumerRoutine()
{
    while(1)
    {
        char name[16];
        pthread_getname_np(pthread_self(), name, sizeof name);
        int out;
        rq.Pop(&out);
        {
            LockGuard lg1(lock1);
        std::cout << "线程" << name << " 消费数据" << out << std::endl;
        }
        sleep(1);
    }
}

int main()
{
    Thread productor1(ProductorRoutine);
    Thread productor2(ProductorRoutine);
    Thread productor3(ProductorRoutine);

    Thread consumer1(ConsumerRoutine);
    Thread consumer2(ConsumerRoutine);

    productor1.Start();
    productor2.Start();
    productor3.Start();
    consumer1.Start();
    consumer2.Start();

    productor1.Join();
    productor2.Join();
    productor3.Join();
    consumer1.Join(); 
    consumer2.Join(); 
    return 0;
}