#pragma once

#include "il2cppObject.h"

namespace il2cpp
{
    struct int32 : object
    {
        int m_iValue;
    };

    struct int64 : object
    {
        long m_lValue;
    };

    struct floatNum : object
    {
        float m_fValue;
    };

    struct doubleNum : object
    {
        double m_dValue;
    };

    struct byteNum : object
    {
        byte m_bValue;
    };

    struct boolean : object
    {
        bool m_bValue;
    };
}
