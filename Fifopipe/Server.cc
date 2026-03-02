#include "Fifo.hpp"
int main()
{
    // 服务端 创建并打开管道
    Fifo pipefile;
    pipefile.Build();
    pipefile.Open(FORREAD);

    std::string msg;
    while (1)
    {
        int n = pipefile.Receive(msg);
        if (n > 0)
        {
            std::cout << "客户端说: " << msg << std::endl;
        }
        else
        {
            break;
        }
    }

    pipefile.Delete();

    return 0;
}