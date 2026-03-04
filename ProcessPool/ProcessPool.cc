#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
// 进程池，是指提前创建好多个子进程，在需要使用时直接分配任务。省去了创建子进程的开销
// 父进程需要管理“通道”，组织管理子进程

enum
{
    OK = 0,
    PIPE_ERR,
    FORK_ERR,
    READ_ERR,
    WRITE_ERR,
    WAIT_ERR
};

// 全局定义好子进程数量，任务数量
const int gprocessnum = 7;

void task1()
{
    std::cout << "这是下载数据任务" << std::endl;
}
void task2()
{
    std::cout << "这是打印日志任务" << std::endl;
}
void task3()
{
    std::cout << "这是刷新磁盘任务" << std::endl;
}
void task4()
{
    std::cout << "这是更新用户状态任务" << std::endl;
}

typedef void (*task_t)();
// 任务表
const int gtasknum = 4;
task_t tasks[gtasknum] = {task1, task2, task3, task4};

class ProcessPool
{
private:
    // 内部类维护子进程通道
    class Channel
    {
    private:
        int _wfd;                 // 当前子进程通道的管道写端fd
        pid_t _id;                // 子进程id
        std::string channel_name; // 自定义通道的名字
    public:
        Channel(int wfd, pid_t id) : _wfd(wfd), _id(id)
        {
            channel_name = "channel-" + std::to_string(id);
        }

        void ClosePipe()
        {
            close(_wfd);
        }

        void PrintInfo()
        {
            printf("管道wfd: %d, 通道名: %s\n", _wfd, channel_name.c_str());
        }

        int getfd()
        {
            return _wfd;
        }

        const char* getname()
        {
            return channel_name.c_str();
        }

        void Wait()
        {
            pid_t rid = waitpid(_id, nullptr, 0);
            if (rid < 0)
            {
                // wait出错
                exit(WAIT_ERR);
            }
            std::cout << "回收子进程: " << _id << std::endl;
        }
    };

public:
    ProcessPool()
    {
        // 种下随机数种子
        srand(time(NULL));
    }

    // 初始化进程池，创建好若干个子进程通道
    void Init()
    {
        CreateProcessChannels();
    }

    // 打印通道信息验证
    void Debug()
    {
        for (auto& channel : channels)
        {
            channel.PrintInfo();
        }
    }

    // 随机分配任务执行
    void Run()
    {
        int cnt = 10;
        while (cnt--)
        {
            int itask = SelectTask();
            int ichannel = SelectChannel();

            printf("向通道%s发送任务task%d...\n", channels[ichannel].getname(), itask + 1);
            SendTask2Channel(itask, ichannel);

            sleep(1);
        }
    }

    void Quit()
    {
        for (auto& channel : channels)
        {
            channel.ClosePipe();
            channel.Wait();
        }
    }

private:
    void CreateProcessChannels()
    {
        for (int i = 0; i < gprocessnum; i++)
        {
            int pipefd[2] = {0};
            int n = pipe(pipefd);
            if (n < 0)
            {
                // 管道创建出错
                exit(PIPE_ERR);
            }

            pid_t id = fork();
            if (id < 0)
            {
                // 子进程创建出错
                exit(FORK_ERR);
            }
            else if (id == 0)
            {
                // 子进程 read
                // 子进程的文件描述符表是拷贝父进程的。父进程fd表中有指向其他管道的wfd，子进程必须关闭指向其他管道的wfd！
                if (!channels.empty())
                {
                    for (auto& channel : channels)
                        channel.ClosePipe();
                }

                // 子进程从管道中读数据，关闭写端
                close(pipefd[1]);

                // 子进程进入待执行任务状态，将来执行完成后回来退出
                DoTask(pipefd[0]);
                exit(OK);
            }
            else
            {
                // 父进程 write
                // 关闭管道读端
                close(pipefd[0]);
                channels.emplace_back(pipefd[1], id);
                printf("创建子进程%d成功\n", id);
            }
        }
    }

    void DoTask(int fd)
    {
        // 子进程需要持续监听管道，等待父进程下发任务，直到父进程主动关闭管道写端。
        while (1)
        {
            int task_code;
            ssize_t n = read(fd, &task_code, sizeof(task_code));
            if (n == sizeof(task_code))
            {
                // 根据读取到的task_code从任务表中选择函数执行
                if (task_code >= 0 && task_code < gtasknum)
                {
                    tasks[task_code]();
                }
            }
            else if (n == 0)
            {
                // 读到了文件尾，说明管道写端关闭了
                printf("%d任务退出\n", getpid());
                break;
            }
            else
            {
                // read出错
                exit(READ_ERR);
            }
        }
    }

    int SelectTask()
    {
        // 随机选一个任务
        return rand() % gtasknum;
    }

    int SelectChannel()
    {
        // 依次选择子进程
        static int i = 0;
        int selected = i;
        i++;
        i %= gprocessnum;
        return selected;
    }

    void SendTask2Channel(int itask, int ichannel)
    {
        assert(0 <= itask && itask < gtasknum && ichannel >= 0 && ichannel < gprocessnum);

        ssize_t n = write(channels[ichannel].getfd(), &itask, sizeof(itask));
        if (n < 0)
        {
            // write出错
            exit(WRITE_ERR);
        }
    }

private:
    // 组织所有的子进程通道
    std::vector<Channel> channels;
};

int main()
{
    ProcessPool pp;
    // 初始化进程池，创建好若干个子进程通道
    pp.Init();

    // 打印通道信息验证
    pp.Debug();

    // 随机分配任务执行
    pp.Run();

    // 释放管道，回收子进程
    pp.Quit();

    return 0;
}