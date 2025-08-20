#include "util.h"
#include <windows.h>
#include <iostream>
#include <list>
#include <vector>

namespace util
{
    bool replace(std::string& str, const std::string& from, const std::string& to)
    {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    bool show_console_window()
    {
        AllocConsole();
        FILE* file = nullptr;
        freopen_s(&file, "CONOUT$", "w", stdout);
        freopen_s(&file, "CONOUT$", "w", stderr);
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        return true;
    }


    std::string clean(const std::string& str)
    {
        if (str.size() > 524288)
        {
            return "";
        }
        std::vector bytes(str.begin(), str.end());
        bytes.push_back('\0');
        std::list<char> chars;
        for (byte byte : bytes)
        {
            if (byte)
            {
                chars.push_back(static_cast<char>(byte));
            }
        }
        std::string clean(chars.begin(), chars.end());
        return clean;
    }
}
