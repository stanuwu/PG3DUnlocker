#pragma once
#include <iostream>
#include <string>

#include "wsSerial.h"
#include "../sdk/il2cppApi.h"
#include "../sdk/il2cppDict.h"
#include "../sdk/il2cppNumeric.h"
#include "../sdk/il2cppList.h"
#include "../sdk/il2cppString.h"
#include "../util/util.h"

class wsSerialNode
{
    wsSerialNode(wsSerial::serialType type, il2cpp::object* nativePtr, int depth) : type(type), nativePtr(nativePtr),
        depth(depth)
    {
    };

public:
    wsSerial::serialType type;
    il2cpp::object* nativePtr;
    int depth;

    static wsSerialNode fromNative(il2cpp::object* object, int depth = 0)
    {
        return {wsSerial::fromObject(object), object, depth};
    }

    static wsSerialNode fromString(std::string string)
    {
        il2cpp::string* nativeString = il2cpp::CreateManagedString(string);
        return fromNative(nativeString);
    }

    std::string toString()
    {
        switch (type)
        {
        case wsSerial::INT:
            return std::to_string(static_cast<il2cpp::int32*>(nativePtr)->m_iValue);
        case wsSerial::LONG:
            return std::to_string(static_cast<il2cpp::int64*>(nativePtr)->m_lValue);
        case wsSerial::FLOAT:
            return std::to_string(static_cast<il2cpp::floatNum*>(nativePtr)->m_fValue);
        case wsSerial::DOUBLE:
            return std::to_string(static_cast<il2cpp::doubleNum*>(nativePtr)->m_dValue);
        case wsSerial::BOOL:
            return std::to_string(static_cast<il2cpp::boolean*>(nativePtr)->m_bValue);
        case wsSerial::STRING:
            {
                auto string = static_cast<il2cpp::string*>(nativePtr);
                if (wsSerial::ptrInvalid(string->m_wString)) return "null";
                return "\"" + util::clean(string->ToString()) + "\"";
            }
        case wsSerial::LIST:
            {
                std::string value = "\n" + getPrefix() + "[\n";
                auto list = static_cast<il2cpp::list<il2cpp::object*>*>(nativePtr);
                if (wsSerial::ptrInvalid(list->m_pListArray)) return "null";
                auto listItems = list->m_pListArray;
                if (wsSerial::ptrInvalid(listItems->m_pValues)) return "null";
                auto listLength = listItems->m_uMaxLength;
                for (int i = 0; i < listLength; ++i)
                {
                    value += getPrefix();
                    value += "  ";
                    auto item = listItems->At(i);
                    if (wsSerial::ptrInvalid(item)) value += "null";
                    else value += fromNative(item, depth + 1).toString();
                    if (i != listLength - 1)
                    {
                        value += ",";
                    }
                    value += "\n";
                }
                value += getPrefix() + "]";
                return value;
            }
        case wsSerial::ARRAY:
            {
                std::string value = "\n" + getPrefix() + "[\n";
                auto listItems = static_cast<il2cpp::array<il2cpp::object*>*>(nativePtr);
                if (wsSerial::ptrInvalid(listItems->m_pValues)) return "null";
                auto listLength = listItems->m_uMaxLength;
                for (int i = 0; i < listLength; ++i)
                {
                    value += getPrefix();
                    auto item = listItems->At(i);
                    if (wsSerial::ptrInvalid(item)) value += "null";
                    else value += fromNative(item, depth + 1).toString();
                    if (i != listLength - 1)
                    {
                        value += ",";
                    }
                    value += "\n";
                }
                value += getPrefix() + "]";
                return value;
            }
        case wsSerial::DICT:
            {
                std::string value = "\n" + getPrefix() + "{\n";
                auto dict = static_cast<il2cpp::dict<il2cpp::object*, il2cpp::object*>*>(nativePtr);
                if (wsSerial::ptrInvalid(dict->m_pEntries)) return "null";
                auto dictLength = dict->m_iCount;
                for (int i = 0; i < dictLength; i++)
                {
                    value += getPrefix();
                    auto key = dict->GetKeyByIndex(i);
                    if (wsSerial::ptrInvalid(key)) value += "null";
                    else value += fromNative(key, depth + 1).toString();
                    value += ": ";
                    auto target = dict->GetValueByIndex(i);
                    if (wsSerial::ptrInvalid(target)) value += "null";
                    else value += fromNative(target, depth + 1).toString();
                    if (i != dictLength - 1)
                    {
                        value += ",";
                    }
                    value += "\n";
                }
                value += getPrefix() + "}";
                return value;
            }
        case wsSerial::NONE:
            return "null";
        default:
            return getPrefix() + nativePtr->type->m_pName;
        }
    }

    std::string getPrefix()
    {
        std::string pref = "";
        for (int i = 0; i < depth; i++)
        {
            pref += "   ";
        }
        return pref;
    }
};
