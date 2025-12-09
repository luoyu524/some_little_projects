#include<stdio.h>
#include<string>
#include<unistd.h>
#include<vector>
#include<sys/types.h>
#include<sys/wait.h>

typedef void(*callback_t)();

void xi()
{
    printf("我是子进程%d, 我是【昔涟】\n", getpid());
    sleep(1);
}

void bai()
{
    printf("我是子进程%d, 我是【白厄】\n", getpid());
    sleep(1);
}

void na()
{
    printf("我是子进程%d, 我是【那刻夏】\n", getpid());
    sleep(1);
}

void a()
{
    printf("我是子进程%d, 我是【阿格莱雅】\n", getpid());
    sleep(1);
}
void sai()
{
    printf("我是子进程%d, 我是【赛飞儿】\n", getpid());
    sleep(1);
}
void feng()
{
    printf("我是子进程%d, 我是【风堇】\n", getpid());
    sleep(1);
}
void chang()
{
    printf("我是子进程%d, 我是【长夜月】\n", getpid());
    sleep(1);
}
void ti()
{
    printf("我是子进程%d, 我是【缇宝】\n", getpid());
    sleep(1);
}
void wan()
{
    printf("我是子进程%d, 我是【万敌】\n", getpid());
    sleep(1);
}
void hai()
{
    printf("我是子进程%d, 我是【海瑟音】\n", getpid());
    sleep(1);
}
void ke()
{
    printf("我是子进程%d, 我是【刻律德菈】\n", getpid());
    sleep(1);
}
void huang()
{
    printf("我是子进程%d, 我是【荒笛】\n", getpid());
    sleep(1);
}

std::vector<callback_t> cbs = {bai, a, sai, ti, wan, feng, na, ke, hai, chang, huang, xi};

void CreatChildProcs(int num, std::vector<pid_t>& childprocs, std::vector<callback_t>& cbs)
{
    for(int i = 0; i < num; i++)
    {
        pid_t id = fork();
        if(id == 0)
        {
            // child
            cbs[i%cbs.size()]();
            exit(0);
        }
        else
        {
            childprocs.push_back(id);
        }
    }
}

void WaitChildProcs(std::vector<pid_t> v)
{
    for(auto id : v)
    {
        int status = 0;
        pid_t rid = waitpid(id, &status, 0);
        if(rid > 0)
        {
            printf("子进程%d已回收，退出码%d\n", rid, WEXITSTATUS(status));
        }
    }
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("你应该传一个数字，表示想要创建的子进程数量\n");
    }
    else if(argc > 2)
    {
        printf("错误！你只能传一个数字\n");
    }
    else
    {
        int num = std::stoi(argv[1]);
        // 创建num个子进程
        std::vector<pid_t> childprocs(num);
        CreatChildProcs(num, childprocs, cbs);

        // 回收这些子进程
        WaitChildProcs(childprocs);
    }
    return 0;
}

