#include <iostream>
#include <thread>
#include <windows.h>

#include "core/offsets.h"
#include "features/unlock.h"
#include "features/ws.h"
#include "menu/dxhook.h"
#include "util/util.h"

void init()
{
    util::show_console_window();
    std::cout << "Dumping\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    bool status = offsets::init();
    if (status)
    {
        std::cout << "OFFSETS OK\n";
    }
    else
    {
        std::cout << "OFFSETS ERROR\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    status = ws::init();
    if (status)
    {
        std::cout << "WS OK\n";
    }
    else
    {
        std::cout << "WS ERROR\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    status = unlock::init();
    if (status)
    {
        std::cout << "UNLOCK OK\n";
    }
    else
    {
        std::cout << "UNLOCK ERROR\n";
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    status = dxhook::init();
    if (status)
    {
        std::cout << "DXHOOK OK\n";
    }
    else
    {
        std::cout << "DXHOOK ERROR\n";
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    static std::thread init_thread(init);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        init_thread.detach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        if (init_thread.joinable())
        {
            init_thread.join();
        }
        break;
    default: ;
    }

    return TRUE;
}
