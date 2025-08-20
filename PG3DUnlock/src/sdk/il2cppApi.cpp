#include "il2cppApi.h"

#include <cstring>
#include <Psapi.h>

#include "il2cppArray.h"
#include "il2cppList.h"
#include "il2cppObject.h"
#include "il2cppString.h"
#include "../core/offsets.h"

namespace il2cpp
{
    void* GetUtf8Encoding()
    {
        return reinterpret_cast<void*(__fastcall*)()>(
            offsets::get_utf8_encoding.GetAddress())();
    }

    string* CreateStringFromEncoding(uint8_t* start, size_t size, void* encoding)
    {
        return reinterpret_cast<string*(__fastcall*)(uint8_t*, size_t, void*, void*)>(
            offsets::create_string_from_encoding.GetAddress())(start, size, encoding, nullptr);
    }

    string* CreateManagedString(std::string string)
    {
        void* encoding = GetUtf8Encoding();
        return CreateStringFromEncoding((uint8_t*)&string.at(0), string.size(), encoding);
    }
}
