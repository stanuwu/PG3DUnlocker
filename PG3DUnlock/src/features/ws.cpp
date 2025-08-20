#include "ws.h"

#include <windows.h>

#include "wsSerialNode.h"
#include "detours/detours.h"
#include "../core/offsets.h"
#include "../sdk/il2cppApi.h"
#include "../sdk/il2cppArray.h"
#include "../sdk/il2cppDict.h"

namespace ws
{
    bool init()
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        oEmit = reinterpret_cast<void* (*)(void*, void*, void*)>(offsets::emit.GetAddress());

        DetourAttach(&(PVOID&)(oEmit), hkEmit);

        LONG status = DetourTransactionCommit();
        if (status != NO_ERROR)
        {
            return false;
        }

        return true;
    }


    void*__fastcall hkEmit(void* socket, void* name, void* args)
    {
        if (logWebsocket)
        {
            auto string = static_cast<il2cpp::string*>(name);
            std::cout << "emit " << std::hex << string->ToString() << "\n";
            auto arg = static_cast<il2cpp::array<il2cpp::dict<il2cpp::string*, il2cpp::object*>*>*>(args);
            std::cout << wsSerialNode::fromNative(arg).toString() << "\n";
        }
        return oEmit(socket, name, args);
    }
}
