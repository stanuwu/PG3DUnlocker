#pragma once

namespace ws
{
    bool init();
    inline bool logWebsocket = false;
    inline void* (*oEmit)(void* socket, void* name, void* args);
    void* __fastcall hkEmit(void* socket, void* name, void* args);
}
