// WingetService.h

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "AppInfo.h"

enum class InstallState
{
    Queued,
    Downloading,
    Installing,
    Completed
};

struct InstallProgressInfo
{
    InstallState state;
    double percent = 0.0;
    uint64_t downloadedBytes = 0;
    uint64_t totalBytes = 0;
};

class WingetService
{
  public:
    using ProgressCallback = std::function<void(const InstallProgressInfo &)>;

    std::vector<AppInfo> SearchApps(const std::wstring &query) const;

    void InstallApp(const std::wstring &packageId, ProgressCallback onProgress = {}) const;
};