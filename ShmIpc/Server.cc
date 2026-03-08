#include "shm.hpp"
int main()
{
    Shm share_mm;
    share_mm.Creat();
    share_mm.Attach();

    char* start_add = (char*)share_mm.GetAdd();
    int size = share_mm.size();

    while (1)
    {
        for (int i = 0; i < size; i++)
        {
            std::cout << start_add[i] << ' ';
        }
        std::cout << std::endl;
        sleep(1);
    }

    share_mm.Dettch();
    share_mm.Delete();

    return 0;
}