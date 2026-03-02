#include "Fifo.hpp"
int main()
{
    // 客户端 写入信息
    Fifo fileclient;
    fileclient.Open(FORWRITE);

    while (1)
    {
        std::cout << "请输入:" << std::endl;
        std::string msg;
        std::getline(std::cin, msg);
        fileclient.Send(msg);
    }
    return 0;
}