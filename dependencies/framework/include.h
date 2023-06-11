#ifndef __framework
#define __framework

#define WIN32
#ifdef WIN32
    #include<Windows.h>
#endif
#include<string>

#include"thread.h" // fxx thread

// ENTRY
#ifdef WIN32
    #define fxx_entry \
        int DllMain(HMODULE i,int r,void*)

    #define fxx_inner \
        if (r != 1) \
            return false;
#else
    #define fxx_entry \
        int __attribute__((constructor)) main()

    #define fxx_inner \
        if (true)
#endif


#endif // __framework

/*
example:

fxx_entry
{
    fxx::entry_thread([](void*) -> uintptr_t{

    })
}

*/