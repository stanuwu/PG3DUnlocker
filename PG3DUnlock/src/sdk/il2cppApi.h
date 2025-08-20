#pragma once
#include "il2cppArray.h"
#include "il2cppList.h"
#include "il2cppObject.h"
#include "il2cppString.h"

namespace il2cpp
{
    string* CreateManagedString(std::string string);
    clazz* FindClass(const char* m_pName);
};
