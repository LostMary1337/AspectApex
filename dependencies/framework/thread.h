#include<functional>
#include<thread>

namespace fxx
{
    inline void entry_thread(std::function <void()> l)
    {
        auto x = std::thread(l);
        x.detach();
    }
}