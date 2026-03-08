#include"shm.hpp"
int main()
{
    Shm share_mm;
    share_mm.Get();
    share_mm.Attach();

    char* start_add = (char*)share_mm.GetAdd();
    int size = share_mm.size();

    int index = 0;
    while (1)
    {
        std::cout << "Please Enter@ ";
        std::cin >> start_add[index];
        index++;
        index %= size;
    }

    share_mm.Dettch();
    
    return 0;
}