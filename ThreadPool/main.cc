#include"Logger.hpp"
#include"ThreadPool.hpp"
#include"Task.hpp"
#include <cstdlib>
#include <ctime>

int main()
{
    srand(time(nullptr));
    USE_CONSOLE_LOG_STRATEGY();
    
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));
    ThreadPool<Task>::Instance()->Enqueue(Task(rand()%100, rand()%100));

    ThreadPool<Task>::Instance()->Stop();
    ThreadPool<Task>::Instance()->Wait();

    




    return 0;
}