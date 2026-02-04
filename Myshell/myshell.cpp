#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<string>
#include<ctype.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/wait.h>

#define MAXSIZE 128
#define MAXARGS 32

// 全局的环境变量表
char* genv[MAXARGS];
int genvc = 0;

// 宏定义重定向方式: 没有 输入重定向 追加重定向 输出重定向
#define NoneRedir 0 
#define InputRedir 1
#define AppRedir 2
#define OutputRedir 3

// 全局变量记录当前重定向方式和目标文件
int redir_type = NoneRedir;
char* redir_filename = NULL;

// 跳过目标文件前的若干空格
#define TrimSpace(start) do{\
    while(isspace(*start)) start++;\
}while(0)

// 记录当前输入命令的命令行参数，输入时以空格分割
char* gargv[MAXARGS];
int gargc = 0;
const char* gsep = " ";

// 我们shell自己所处的工作路径
char cwd[MAXSIZE];

// 最近一个进程的退出码
int lastcode = 0;



void LoadEnv();
void PrintCommandLine();
size_t GetCommand(char* command_line, size_t size);
void ParseRedirect(char* command_line);
void ParseCommand(char* command_line);
bool CheckBuiltInCommand();
void ExecuteCommand();

int main()
{
    // 1.配置环境变量表
    LoadEnv();

    // command_line记录输入的命令字符串内容
    char command_line[MAXSIZE] = {0};

    while(1) // 除非自己中断进程，否则一直循环执行
    {
        // 2.打印命令行shell信息
        PrintCommandLine();

        // 3.获取输入的命令，如果什么都没输入就continue
        if(GetCommand(command_line, sizeof(command_line)) == 0)
        {
            continue;
        }

        // 4.分析重定向情况
        ParseRedirect(command_line);
        
        // 5.解析命令字符串，解析成命令行参数记录到全局的命令行参数表
        ParseCommand(command_line);
        
        // 6.检查是否为内建命令，如果是，让shell自己处理
        if(CheckBuiltInCommand())
        {
           continue;
        }

        // 7.是普通命令，创建子进程执行它
        ExecuteCommand();
    }

    return 0;
}



void LoadEnv()
{
    // 正常情况下环境变量是从配置文件中获取的
    // 但是现在我们直接从父进程拷贝环境变量表
    extern char** environ;
    while(environ[genvc])
    {
        genv[genvc] = (char*)malloc(sizeof(char)*4096);
        strcpy(genv[genvc], environ[genvc]);
        genvc++;
    }
    genv[genvc] = NULL;

    // 我们让启动shell进程时，打印出环境变量信息
    printf("Load Env:\n");
    for(int i = 0; i < genvc; i++)
    {
        printf("%s\n", genv[i]);
    }
}


const char* GetUserName()
{
    char* user_name = getenv("USER");
    if(user_name == NULL)
    {
        return "None";
    }
    return user_name;
}

const char* GetHostName()
{

    char* host_name = getenv("HOSTNAME");
    if(host_name == NULL)
    {
        return "None";
    }
    return host_name;
}

static const char* rfindDirctory(const std::string& s)
{
    if(s == "/")
    {
        return s.c_str();
    }

    auto pos = s.rfind("/");
    if(pos == std::string::npos)
    {
        return s.c_str();
    }
    return s.substr(pos+1).c_str();
}

const char* GetPwd()
{
    char* pwd = getenv("PWD");
    if(pwd == NULL)
    {
        return "None";
    }

    // 如果当前目录是家目录，就显示成~
    if(strcmp(pwd, getenv("HOME")) == 0)
    {
        return "~";
    }

    // 路径以/分割，截取路径的最后一段
    return rfindDirctory(pwd);
}

void PrintCommandLine()
{
    // 我们想让自己的shell命令行格式是：
    // [用户名@主机 当前路径的最后一段]#
    printf("[%s@%s %s]# ", GetUserName(), GetHostName(), GetPwd());
    
    //想立即显示，需要刷新一下缓冲区
    fflush(stdout);
}

size_t GetCommand(char* command_line, size_t size)
{
    // fgets遇到\n换行符时会停止读取，但\n也会被记录进去
    if(fgets(command_line, size, stdin) == NULL)
    {
        return 0;
    }
    // 一行命令，用户一定会至少输入一个\n，比如用户在shell中输入执行ls，实际上读取成l s \n \0
    // 将\n字符置为\0，让输入的内容成为一个标准的C风格字符串
    command_line[strlen(command_line)-1] = '\0';

    return strlen(command_line);
}

void ParseRedirect(char* command_line)
{
    redir_type = NoneRedir;
    redir_filename = NULL;
    char* start = command_line;
    char* end = start + strlen(command_line);

    while(start < end)
    {
        if(*start == '>')
        {   
            if(*(start+1) == '>')
            {
                // 追加重定向
                // 重定向符设为\0，不影响之后解析命令，下同
                *start = '\0';
                start++;
                *start = '\0';
                start++;
                // 跳过目标文件前的若干空格，下同
                TrimSpace(start);

                redir_type = AppRedir;
                redir_filename = start;
                break;
            }
            else
            {
                // 输出重定向   
                *start = '\0';
                start++;
                TrimSpace(start);

                redir_type = OutputRedir;
                redir_filename = start;
                break;
            }
        }
        else if(*start == '<')
        {
            // 输入重定向
            *start = '\0';
            start++;
            TrimSpace(start);

            redir_type = InputRedir;
            redir_filename = start;
            break;
        }
        else
        {
            start++;
        }
    }
}

void ParseCommand(char* command_line)
{
    // 初始化grav和gravc
    gargc = 0;
    memset(gargv, 0, sizeof(gargv));
     
    gargv[0] = strtok(command_line, gsep);
    while((gargv[++gargc] = strtok(NULL, gsep)));
}

bool CheckBuiltInCommand()
{
    // 如果不是内建命令，返回false
    // 如果是内建命令，执行，返回true
    // 这里演示处理的内建命令：cd, echo $?, echo $PATH
    
    if(strcmp(gargv[0], "cd") == 0)
    {
        if(gargc == 2)
        {
            // 1.更改当前进程工作目录
            chdir(gargv[1]);

            // 2.更改环境变量
            char pwd[1024];
            getcwd(pwd, sizeof(pwd)); // pwd记录现在工作路径
            snprintf(cwd, sizeof(cwd), "PWD=%s", pwd); // cwd的格式是“PWD=绝对路径”
            putenv(cwd); // 写入环境变量表中

            lastcode = 0;   
        }
        return true;
    }
    else if(strcmp(gargv[0], "echo") == 0)
    {
        if(gargc == 2)
        {
            if(gargv[1][0] == '$') // echo $? 查询上一个进程的退出码
            {
                if(strcmp(gargv[1]+1, "?") == 0)
                {
                    printf("lastcode: %d\n", lastcode);
                }
                else if(strcmp(gargv[1]+1, "PATH") == 0) // echo $PATH 查询环境变量PATH内容
                {
                    printf("%s\n", getenv("PATH")); 
                }

                lastcode = 0;
            }
            return true;
        }
    }

    return false;
}

void ExecuteCommand()
{
    pid_t id = fork();
    if(id == -1)
    {
        exit(2);
    }
    else if(id == 0)
    {
        // 子进程完成重定向、进程替换执行命令
        int fd = -1;
        if(redir_type == InputRedir)
        {
            fd = open(redir_filename, O_RDONLY);
            dup2(fd, 0);
        }
        else if(redir_type == OutputRedir)
        {
            fd = open(redir_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            dup2(fd, 1);
        }
        else if(redir_type == AppRedir)
        {
            fd = open(redir_filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
            dup2(fd, 1);
        }

        execvpe(gargv[0], gargv, genv);

        exit(1);
    }
    else
    {
        // 父进程等待子进程回收
        int status = 0;
        pid_t retpid = waitpid(id, &status, 0);
        if(retpid > 0)
        {
            lastcode = WEXITSTATUS(status);
        }
    }
}
















