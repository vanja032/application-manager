// AppInfo.h

#pragma once

#include <string>

struct AppInfo
{
    std::wstring id;
    std::wstring name;
    bool installed = false;
    std::wstring installedVersion;
};