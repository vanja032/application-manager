// WingetService.h

#pragma once

#include "AppInfo.h"

#include <string>
#include <vector>

class WingetService
{
  public:
    std::vector<AppInfo> SearchApps(const std::wstring &query) const;
};