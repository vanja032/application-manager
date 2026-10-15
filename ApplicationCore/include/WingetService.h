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
    std::wstring installedVersion;
};

struct UninstallCompletion
{
    std::string error;
};

struct UpdateCompletion
{
    std::string error;
    std::wstring installedVersion;
};

struct LaunchCompletion
{
    std::string error;
    std::vector<std::wstring> appUserModelIds;
};

class WingetService
{
  public:
    using ProgressCallback = std::function<void(const InstallProgressInfo &)>;
    using UninstallProgressCallback = std::function<void(const UninstallProgressInfo &)>;
    using UpdateProgressCallback = std::function<void(const UpdateProgressInfo &)>;
    using SearchCallback = std::function<void(SearchCompletion)>;
    using InstallCallback = std::function<void(InstallCompletion)>;
    using UninstallCallback = std::function<void(UninstallCompletion)>;
    using UpdateCallback = std::function<void(UpdateCompletion)>;
    using LaunchCallback = std::function<void(LaunchCompletion)>;

    WingetService();
    ~WingetService();

    WingetService(const WingetService &) = delete;
    WingetService &operator=(const WingetService &) = delete;
    WingetService(WingetService &&) = delete;
    WingetService &operator=(WingetService &&) = delete;

    std::vector<AppInfo> SearchApps(const std::wstring &query) const;
    void SearchAppsAsync(const std::wstring &query, SearchCallback callback);

    std::vector<AppInfo> ListFeaturedApps() const;
    void ListFeaturedAppsAsync(SearchCallback callback);

    std::vector<AppInfo> ListInstalledApps() const;
    void ListInstalledAppsAsync(SearchCallback callback);

    std::vector<AppInfo> ListUpdates() const;
    void ListUpdatesAsync(SearchCallback callback);

    void InstallApp(const std::wstring &packageId, ProgressCallback onProgress = {}) const;
    void InstallAppAsync(const std::wstring &packageId, ProgressCallback onProgress, InstallCallback onComplete);

    void UninstallApp(const std::wstring &packageId, UninstallProgressCallback onProgress = {}) const;
    void UninstallAppAsync(const std::wstring &packageId, UninstallProgressCallback onProgress, UninstallCallback onComplete);

    void UpdateApp(const std::wstring &packageId, UpdateProgressCallback onProgress = {}) const;
    void UpdateAppAsync(const std::wstring &packageId, UpdateProgressCallback onProgress, UpdateCallback onComplete);

    std::string ActivateAppUserModelIds(const std::vector<std::wstring> &appUserModelIds) const;
    void LaunchAppAsync(const std::wstring &packageId, LaunchCallback callback);

  private:
    enum class CatalogRequest
    {
        Search,
        Featured,
        Installed,
        Updates,
        Launch
    };

    struct SearchRequest
    {
        CatalogRequest kind = CatalogRequest::Search;
        std::wstring query;
        SearchCallback callback;
        LaunchCallback onLaunch;
    };

    struct InstallRequest
    {
        std::wstring packageId;
        ProgressCallback onProgress;
        InstallCallback onComplete;
    };

    struct UninstallRequest
    {
        std::wstring packageId;
        UninstallProgressCallback onProgress;
        UninstallCallback onComplete;
    };

    struct UpdateRequest
    {
        std::wstring packageId;
        UpdateProgressCallback onProgress;
        UpdateCallback onComplete;
    };

    void workerMain();
    std::vector<AppInfo> listStorePackages(bool updatesOnly) const;
    void installWorkerMain();
    void uninstallWorkerMain();
    void updateWorkerMain();
    std::wstring installPackage(const std::wstring &packageId, ProgressCallback onProgress,
                                std::function<void(std::function<void()>)> armCancel) const;
    void uninstallPackage(const std::wstring &packageId, UninstallProgressCallback onProgress,
                          std::function<void(std::function<void()>)> armCancel) const;
    std::wstring updatePackage(const std::wstring &packageId, UpdateProgressCallback onProgress,
                               std::function<void(std::function<void()>)> armCancel) const;
    std::vector<std::wstring> findAppUserModelIds(const std::wstring &packageId) const;

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

    std::mutex m_uninstallMutex;
    std::condition_variable m_uninstallCv;
    std::deque<UninstallRequest> m_uninstallQueue;
    bool m_uninstallStop = false;
    std::wstring m_activeUninstallId;
    std::function<void()> m_cancelUninstall;

    std::mutex m_updateMutex;
    std::condition_variable m_updateCv;
    std::deque<UpdateRequest> m_updateQueue;
    bool m_updateStop = false;
    std::wstring m_activeUpdateId;
    std::function<void()> m_cancelUpdate;

    std::thread m_updateWorker;
    std::thread m_uninstallWorker;
    std::thread m_installWorker;
    std::thread m_worker;
};