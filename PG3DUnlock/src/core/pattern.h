#pragma once
#include <string>
#include <vector>
#include <windows.h>
#include <iostream>
#include <sstream>

class pattern;

class pattern
{
public:
    pattern(const byte offset, const byte pad, const std::string& pattern_, bool deref = true)
        : offset(offset),
          pad(pad),
          deref(deref), address(0), pattern_(pattern_.c_str()), parsed(false)
    {
    }

    ~pattern()
    {
        delete[] patternBytes;
    }

    bool Scan(byte* addressScan)
    {
        if (!parsed)
        {
            ParsePattern();
        }
        if (!ScanPattern(addressScan)) return false;
        if (deref)
        {
            int32_t p = *(int32_t*)((uint64_t)(addressScan + offset));
            this->address = (uint64_t)(addressScan) + p + pad;
            return true;
        }
        this->address = (uint64_t)(addressScan) + offset + pad;
        return true;
    }

    uint64_t GetAddress()
    {
        return this->address;
    }

private:
    byte offset;
    byte pad;
    size_t length;
    byte* patternBytes;
    bool deref;
    uint64_t address;
    std::string pattern_;
    bool parsed;

    bool ScanPattern(byte* addressScan, int index = 0)
    {
        if (index == length) return true;
        if (patternBytes[index] == 0x00 || (void*)patternBytes[index] == (void*)*(addressScan + index))
            return ScanPattern(
                addressScan, index + 1);
        return false;
    }

    void ParsePattern()
    {
        length = (pattern_.length() + 1) / 3;
        this->patternBytes = new byte[length];
        std::istringstream iss(this->pattern_.c_str());
        for (size_t i = 0; i < length; ++i)
        {
            std::string byteStr;
            iss >> byteStr;

            if (byteStr == "??")
            {
                this->patternBytes[i] = 0;
            }
            else
            {
                this->patternBytes[i] = static_cast<byte>(std::stoi(byteStr, nullptr, 16));
            }
        }
        this->parsed = true;
    }
};
