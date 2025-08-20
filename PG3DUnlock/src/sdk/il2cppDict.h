#pragma once
#include "il2cppArray.h"
#include "il2cppObject.h"

namespace il2cpp
{
    template <typename TKey, typename TValue>
    struct dict : object
    {
        struct Entry
        {
            int m_iHashCode;
            int m_iNext;
            TKey m_tKey;
            TValue m_tValue;
        };

        array<int>* m_pBuckets;
        array<Entry*>* m_pEntries;
        int m_iCount;
        int m_iVersion;
        int m_iFreeList;
        int m_iFreeCount;
        void* m_pComparer;
        void* m_pKeys;
        void* m_pValues;

        Entry* GetEntry()
        {
            return static_cast<Entry*>((void*)m_pEntries->GetData());
        }

        TKey GetKeyByIndex(int iIndex)
        {
            TKey tKey = {0};

            Entry* pEntry = GetEntry();
            if (pEntry)
                tKey = pEntry[iIndex].m_tKey;

            return tKey;
        }

        TValue GetValueByIndex(int iIndex)
        {
            TValue tValue = {0};

            Entry* pEntry = GetEntry();
            if (pEntry)
                tValue = pEntry[iIndex].m_tValue;

            return tValue;
        }

        TValue GetValueByKey(TKey tKey)
        {
            TValue tValue = {0};
            for (int i = 0; i < m_iCount; i++)
            {
                if (GetEntry()[i].m_tKey == tKey)
                    tValue = GetEntry()[i].m_tValue;
            }
            return tValue;
        }
    };
}
