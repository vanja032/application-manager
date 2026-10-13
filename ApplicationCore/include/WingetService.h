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
    InstallState state = InstallState::Queued;
    double percent = 0.0;
    uint64_t downloadedBytes = 0;
    uint64_t totalBytes = 0;
};

enum class UninstallState
{
    Queued,
    Uninstalling,
    Completed
};

struct UninstallProgressInfo
{
    UninstallState state = UninstallState::Queued;
    double percent = 0.0;
};

enum class UpdateState
{
    Queued,
    Downloading,
    Updating,
    Completed
};

struct UpdateProgressInfo
{
    UpdateState state = UpdateState::Queued;
    double percent = 0.0;
    uint64_t downloadedBytes = 0;
    uint64_t totalBytes = 0;
};

class WingetService
{
  public:
    using ProgressCallback = std::function<void(const InstallProgressInfo &)>;
    using UninstallProgressCallback = std::function<void(const UninstallProgressInfo &)>;
    using UpdateProgressCallback = std::function<void(const UpdateProgressInfo &)>;

    std::vector<AppInfo> SearchApps(const std::wstring &query) const;

    void InstallApp(const std::wstring &packageId, ProgressCallback onProgress = {}) const;

    void UninstallApp(const std::wstring &packageId, UninstallProgressCallback onProgress = {}) const;

    void UpdateApp(const std::wstring &packageId, UpdateProgressCallback onProgress = {}) const;
};