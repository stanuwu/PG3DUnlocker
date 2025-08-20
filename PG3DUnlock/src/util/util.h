#pragma once
#include <string>

namespace util
{
    bool replace(std::string& str, const std::string& from, const std::string& to);
    bool show_console_window();
    std::string clean(const std::string& str);
}
