#pragma once
#include "il2cppArray.h"

namespace il2cpp
{
    template <typename T>
    struct list : object
    {
        array<T>* m_pListArray;

        array<T>* ToArray() { return m_pListArray; }
    };
}
