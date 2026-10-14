// WingetService.h

#pragma once

#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
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

struct SearchCompletion
{
    std::vector<AppInfo> apps;
    std::string error;
};

struct InstallCompletion
{
    std::string error;
};

class WingetService
{
  public:
    using ProgressCallback = std::function<void(const InstallProgressInfo &)>;
    using UninstallProgressCallback = std::function<void(const UninstallProgressInfo &)>;
    using UpdateProgressCallback = std::function<void(const UpdateProgressInfo &)>;
    using SearchCallback = std::function<void(SearchCompletion)>;
    using InstallCallback = std::function<void(InstallCompletion)>;

    WingetService();
    ~WingetService();

    WingetService(const WingetService &) = delete;
    WingetService &operator=(const WingetService &) = delete;
    WingetService(WingetService &&) = delete;
    WingetService &operator=(WingetService &&) = delete;

    std::vector<AppInfo> SearchApps(const std::wstring &query) const;
    void SearchAppsAsync(const std::wstring &query, SearchCallback callback);

    void InstallApp(const std::wstring &packageId, ProgressCallback onProgress = {}) const;
    void InstallAppAsync(const std::wstring &packageId, ProgressCallback onProgress, InstallCallback onComplete);

    void UninstallApp(const std::wstring &packageId, UninstallProgressCallback onProgress = {}) const;

    void UpdateApp(const std::wstring &packageId, UpdateProgressCallback onProgress = {}) const;

  private:
    struct SearchRequest
    {
        std::wstring query;
        SearchCallback callback;
    };

    struct InstallRequest
    {
        std::wstring packageId;
        ProgressCallback onProgress;
        InstallCallback onComplete;
    };

    void workerMain();
    void installWorkerMain();
    void installPackage(const std::wstring &packageId, ProgressCallback onProgress,
                        std::function<void(std::function<void()>)> armCancel) const;

    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::optional<SearchRequest> m_pending;
    bool m_stop = false;

    std::mutex m_installMutex;
    std::condition_variable m_installCv;
    std::deque<InstallRequest> m_installQueue;
    bool m_installStop = false;
    std::wstring m_activeInstallId;
    std::function<void()> m_cancelActive;
    std::thread m_installWorker;
    std::thread m_worker;
};