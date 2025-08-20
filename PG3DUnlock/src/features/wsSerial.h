#pragma once
#include "../sdk/il2cppArray.h"

namespace wsSerial
{
    enum serialType
    {
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        BOOL,
        STRING,
        LIST,
        ARRAY,
        DICT,
        NONE
    };

    /**
     * Sadly because there is bogous data in il2cpp structures we cant really get around this kind of thing
     * @param ptr 
     * @return 
     */
    inline bool ptrInvalid(void* ptr)
    {
        return IsBadReadPtr(ptr, 8);
    }

    inline serialType fromObject(il2cpp::object* obj)
    {
        if (ptrInvalid(obj) || ptrInvalid(obj->type) || ptrInvalid((void*)obj->type->m_pName)) return NONE;
        if (std::string(obj->type->m_pName) == "Int32") return INT;
        if (std::string(obj->type->m_pName) == "Int64") return LONG;
        if (std::string(obj->type->m_pName) == "Single") return FLOAT;
        if (std::string(obj->type->m_pName) == "Double") return DOUBLE;
        if (std::string(obj->type->m_pName) == "Boolean") return BOOL;
        if (std::string(obj->type->m_pName) == "String") return STRING;
        if (std::string(obj->type->m_pName) == "List`1") return LIST;
        if (std::string(obj->type->m_pName) == "ArrayList") return LIST;
        if (std::string(obj->type->m_pName).ends_with("[]")) return ARRAY;
        if (std::string(obj->type->m_pName) == "Dictionary`2") return DICT;
        if (std::string(obj->type->m_pName) == "Hashtable") return DICT;
        std::cout << "INVALID TYPE: " << obj->type->m_pName << "\n";
        return NONE;
    }
}
