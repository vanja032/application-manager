// WingetService.cpp

#include "WingetService.h"
#include "WingetErrors.h"

#include <winrt/Microsoft.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/base.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace deployment = winrt::Microsoft::Management::Deployment;

namespace
{

struct WorkerApartment
{
    WorkerApartment()
    {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    }

    ~WorkerApartment()
    {
        winrt::uninit_apartment();
    }

    WorkerApartment(const WorkerApartment &) = delete;
    WorkerApartment &operator=(const WorkerApartment &) = delete;
};

std::wstring lookupInstalledVersion(const std::wstring &packageId)
{
    try
    {
        deployment::PackageManager manager;
        auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
        if (!storeCatalog)
        {
            return {};
        }

        deployment::CreateCompositePackageCatalogOptions compositeOptions;
        compositeOptions.Catalogs().Append(storeCatalog);
        compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::RemotePackagesFromAllCatalogs);

        auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
        if (!compositeCatalog)
        {
            return {};
        }

        auto connection = compositeCatalog.Connect();
        if (connection.Status() != deployment::ConnectResultStatus::Ok)
        {
            return {};
        }

        deployment::FindPackagesOptions options;
        deployment::PackageMatchFilter filter;
        filter.Field(deployment::PackageMatchField::Id);
        filter.Option(deployment::PackageFieldMatchOption::Equals);
        filter.Value(packageId);
        options.Selectors().Append(filter);

        auto result = connection.PackageCatalog().FindPackages(options);
        if (result.Status() != deployment::FindPackagesResultStatus::Ok || result.Matches().Size() == 0)
        {
            return {};
        }

        auto installed = result.Matches().GetAt(0).CatalogPackage().InstalledVersion();
        if (!installed)
        {
            return {};
        }

        return installed.Version().c_str();
    }
    catch (...)
    {
        return {};
    }
}

std::wstring versionAfterChange(const deployment::CatalogPackage &package, const std::wstring &packageId)
{
    std::wstring version = lookupInstalledVersion(packageId);
    if (!version.empty())
    {
        return version;
    }

    if (auto available = package.DefaultInstallVersion())
    {
        return available.Version().c_str();
    }

    return {};
}

bool catalogIsStore(const deployment::PackageCatalog &catalog)
{
    if (!catalog)
    {
        return false;
    }

    auto info = catalog.Info();
    if (!info)
    {
        return false;
    }

    return info.Id() == L"msstore" || info.Name() == L"msstore" || info.Argument() == L"msstore";
}

bool versionIsStore(const deployment::PackageVersionInfo &version)
{
    return version && catalogIsStore(version.PackageCatalog());
}

bool isMicrosoftStorePackage(const deployment::CatalogPackage &package)
{
    return versionIsStore(package.DefaultInstallVersion()) || versionIsStore(package.InstalledVersion());
}

AppInfo appFromPackage(const deployment::CatalogPackage &package)
{
    AppInfo app;
    app.id = package.Id().c_str();
    app.name = package.Name().c_str();

    if (auto installedVersion = package.InstalledVersion())
    {
        app.installed = true;
        app.installedVersion = installedVersion.Version().c_str();
    }

    app.updateAvailable = package.IsUpdateAvailable();
    if (auto availableVersion = package.DefaultInstallVersion())
    {
        app.availableVersion = availableVersion.Version().c_str();
    }

    return app;
}

} // namespace

WingetService::WingetService()
    : m_updateWorker(&WingetService::updateWorkerMain, this),
      m_uninstallWorker(&WingetService::uninstallWorkerMain, this),
      m_installWorker(&WingetService::installWorkerMain, this),
      m_worker(&WingetService::workerMain, this)
{
}

WingetService::~WingetService()
{
    {
        std::lock_guard lock(m_mutex);
        m_stop = true;
        m_pending.reset();
    }

    m_cv.notify_one();

    std::function<void()> cancel;
    {
        std::lock_guard lock(m_installMutex);
        m_installStop = true;
        m_installQueue.clear();
        cancel = std::move(m_cancelActive);
    }

    if (cancel)
    {
        cancel();
    }

    m_installCv.notify_all();

    std::function<void()> cancelUninstall;
    {
        std::lock_guard lock(m_uninstallMutex);
        m_uninstallStop = true;
        m_uninstallQueue.clear();
        cancelUninstall = std::move(m_cancelUninstall);
    }

    if (cancelUninstall)
    {
        cancelUninstall();
    }

    m_uninstallCv.notify_all();

    std::function<void()> cancelUpdate;
    {
        std::lock_guard lock(m_updateMutex);
        m_updateStop = true;
        m_updateQueue.clear();
        cancelUpdate = std::move(m_cancelUpdate);
    }

    if (cancelUpdate)
    {
        cancelUpdate();
    }

    m_updateCv.notify_all();

    if (m_updateWorker.joinable())
    {
        m_updateWorker.join();
    }

    if (m_uninstallWorker.joinable())
    {
        m_uninstallWorker.join();
    }

    if (m_installWorker.joinable())
    {
        m_installWorker.join();
    }

    if (m_worker.joinable())
    {
        m_worker.join();
    }
}

void WingetService::workerMain()
{
    const WorkerApartment apartment;

    for (;;)
    {
        SearchRequest request;
        {
            std::unique_lock lock(m_mutex);
            m_cv.wait(lock, [this] { return m_stop || m_pending.has_value(); });

            if (m_stop)
            {
                return;
            }

            request = std::move(*m_pending);
            m_pending.reset();
        }

        SearchCompletion completion;
        try
        {
            if (request.kind == CatalogRequest::Installed)
            {
                completion.apps = ListInstalledApps();
            }
            else if (request.kind == CatalogRequest::Updates)
            {
                completion.apps = ListUpdates();
            }
            else if (request.kind == CatalogRequest::Featured)
            {
                completion.apps = ListFeaturedApps();
            }
            else
            {
                completion.apps = SearchApps(request.query);
            }
        }
        catch (const winrt::hresult_error &ex)
        {
            completion.error = winrt::to_string(ex.message());
        }
        catch (const std::exception &ex)
        {
            completion.error = ex.what();
        }

        if (request.callback)
        {
            request.callback(std::move(completion));
        }
    }
}

void WingetService::SearchAppsAsync(const std::wstring &query, SearchCallback callback)
{
    {
        std::lock_guard lock(m_mutex);
        m_pending = SearchRequest{CatalogRequest::Search, query, std::move(callback)};
    }

    m_cv.notify_one();
}

std::vector<AppInfo> WingetService::SearchApps(const std::wstring &query) const
{
    deployment::PackageManager manager;

    auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
    if (!storeCatalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    deployment::CreateCompositePackageCatalogOptions compositeOptions;
    compositeOptions.Catalogs().Append(storeCatalog);
    compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::RemotePackagesFromAllCatalogs);

    auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
    if (!compositeCatalog)
    {
        throw std::runtime_error("Failed to create composite catalog");
    }

    auto connection = compositeCatalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to composite catalog");
    }

    deployment::FindPackagesOptions options;
    deployment::PackageMatchFilter filter;

    filter.Field(deployment::PackageMatchField::Name);
    filter.Option(deployment::PackageFieldMatchOption::ContainsCaseInsensitive);
    filter.Value(query);

    constexpr std::uint32_t kSearchResultLimit = 10;
    options.Filters().Append(filter);
    options.ResultLimit(kSearchResultLimit);

    auto result = connection.PackageCatalog().FindPackages(options);
    if (result.Status() != deployment::FindPackagesResultStatus::Ok)
    {
        throw std::runtime_error("Failed to search msstore catalog");
    }

    std::vector<AppInfo> apps;

    for (const auto &match : result.Matches())
    {
        apps.push_back(appFromPackage(match.CatalogPackage()));
    }

    return apps;
}

std::vector<AppInfo> WingetService::ListFeaturedApps() const
{
    deployment::PackageManager manager;

    auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
    if (!storeCatalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    deployment::CreateCompositePackageCatalogOptions compositeOptions;
    compositeOptions.Catalogs().Append(storeCatalog);
    compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::RemotePackagesFromAllCatalogs);

    auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
    if (!compositeCatalog)
    {
        throw std::runtime_error("Failed to create composite catalog");
    }

    auto connection = compositeCatalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to composite catalog");
    }

    static constexpr const wchar_t *kFeaturedNames[] = {
        L"WhatsApp", L"Spotify", L"Netflix", L"Instagram", L"TikTok", L"Discord",
        L"Telegram", L"Zoom",    L"Disney",  L"Prime Video", L"Adobe Acrobat", L"Facebook"};

    auto catalog = connection.PackageCatalog();
    std::vector<AppInfo> apps;
    for (const wchar_t *name : kFeaturedNames)
    {
        if (apps.size() >= 10)
        {
            break;
        }

        deployment::FindPackagesOptions options;
        deployment::PackageMatchFilter filter;
        filter.Field(deployment::PackageMatchField::Name);
        filter.Option(deployment::PackageFieldMatchOption::ContainsCaseInsensitive);
        filter.Value(name);
        options.Filters().Append(filter);
        options.ResultLimit(1);

        auto result = catalog.FindPackages(options);
        if (result.Status() != deployment::FindPackagesResultStatus::Ok || result.Matches().Size() == 0)
        {
            continue;
        }

        AppInfo app = appFromPackage(result.Matches().GetAt(0).CatalogPackage());
        if (app.id.empty())
        {
            continue;
        }

        bool seen = false;
        for (const AppInfo &existing : apps)
        {
            if (existing.id == app.id)
            {
                seen = true;
                break;
            }
        }
        if (!seen)
        {
            apps.push_back(std::move(app));
        }
    }

    return apps;
}

void WingetService::ListFeaturedAppsAsync(SearchCallback callback)
{
    {
        std::lock_guard lock(m_mutex);
        m_pending = SearchRequest{CatalogRequest::Featured, {}, std::move(callback)};
    }

    m_cv.notify_one();
}

std::vector<AppInfo> WingetService::ListInstalledApps() const
{
    return listStorePackages(false);
}

void WingetService::ListInstalledAppsAsync(SearchCallback callback)
{
    {
        std::lock_guard lock(m_mutex);
        m_pending = SearchRequest{CatalogRequest::Installed, {}, std::move(callback)};
    }

    m_cv.notify_one();
}

std::vector<AppInfo> WingetService::ListUpdates() const
{
    return listStorePackages(true);
}

void WingetService::ListUpdatesAsync(SearchCallback callback)
{
    {
        std::lock_guard lock(m_mutex);
        m_pending = SearchRequest{CatalogRequest::Updates, {}, std::move(callback)};
    }

    m_cv.notify_one();
}

std::vector<AppInfo> WingetService::listStorePackages(bool updatesOnly) const
{
    deployment::PackageManager manager;

    auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
    if (!storeCatalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    deployment::CreateCompositePackageCatalogOptions compositeOptions;
    compositeOptions.Catalogs().Append(storeCatalog);
    compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::LocalCatalogs);

    auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
    if (!compositeCatalog)
    {
        throw std::runtime_error("Failed to create composite catalog");
    }

    auto connection = compositeCatalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to composite catalog");
    }

    deployment::FindPackagesOptions options;
    auto result = connection.PackageCatalog().FindPackages(options);
    if (result.Status() != deployment::FindPackagesResultStatus::Ok)
    {
        throw std::runtime_error("Failed to list installed packages");
    }

    std::vector<AppInfo> apps;
    for (const auto &match : result.Matches())
    {
        auto package = match.CatalogPackage();
        if (!package.InstalledVersion() || !isMicrosoftStorePackage(package))
        {
            continue;
        }

        AppInfo app = appFromPackage(package);
        if (updatesOnly && !app.updateAvailable)
        {
            continue;
        }

        apps.push_back(std::move(app));
    }

    return apps;
}

void WingetService::InstallApp(const std::wstring &packageId, ProgressCallback onProgress) const
{
    installPackage(packageId, std::move(onProgress), {});
}

void WingetService::InstallAppAsync(const std::wstring &packageId, ProgressCallback onProgress, InstallCallback onComplete)
{
    {
        std::lock_guard lock(m_installMutex);
        if (m_installStop || m_activeInstallId == packageId)
        {
            return;
        }

        for (const InstallRequest &queued : m_installQueue)
        {
            if (queued.packageId == packageId)
            {
                return;
            }
        }

        m_installQueue.push_back(InstallRequest{packageId, std::move(onProgress), std::move(onComplete)});
    }

    m_installCv.notify_one();
}

void WingetService::installWorkerMain()
{
    const WorkerApartment apartment;

    for (;;)
    {
        InstallRequest request;
        {
            std::unique_lock lock(m_installMutex);
            m_installCv.wait(lock, [this] { return m_installStop || !m_installQueue.empty(); });

            if (m_installStop)
            {
                return;
            }

            request = std::move(m_installQueue.front());
            m_installQueue.pop_front();
            m_activeInstallId = request.packageId;
        }

        struct ClearActive
        {
            WingetService *self;

            ~ClearActive()
            {
                std::lock_guard lock(self->m_installMutex);
                self->m_cancelActive = nullptr;
                self->m_activeInstallId.clear();
            }
        } clearActive{this};

        InstallCompletion completion;
        try
        {
            completion.installedVersion = installPackage(
                request.packageId, request.onProgress, [this](std::function<void()> cancel) {
                    std::lock_guard lock(m_installMutex);
                    if (m_installStop)
                    {
                        cancel();
                        return;
                    }

                    m_cancelActive = std::move(cancel);
                });
        }
        catch (const winrt::hresult_error &ex)
        {
            completion.error = winrt::to_string(ex.message());
        }
        catch (const std::exception &ex)
        {
            completion.error = ex.what();
        }

        bool stopped = false;
        {
            std::lock_guard lock(m_installMutex);
            stopped = m_installStop;
        }

        if (stopped)
        {
            return;
        }

        if (request.onComplete)
        {
            request.onComplete(std::move(completion));
        }
    }
}

std::wstring WingetService::installPackage(const std::wstring &packageId, ProgressCallback onProgress,
                                            std::function<void(std::function<void()>)> armCancel) const
{
    deployment::PackageManager manager;

    auto catalog = manager.GetPackageCatalogByName(L"msstore");
    if (!catalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    auto connection = catalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to msstore");
    }

    deployment::FindPackagesOptions options;
    deployment::PackageMatchFilter filter;

    filter.Field(deployment::PackageMatchField::Id);
    filter.Value(packageId);
    options.Selectors().Append(filter);

    auto result = connection.PackageCatalog().FindPackages(options);
    if (result.Status() != deployment::FindPackagesResultStatus::Ok)
    {
        throw std::runtime_error("Failed to search msstore catalog");
    }

    if (result.Matches().Size() == 0)
    {
        throw std::runtime_error("Package not found");
    }

    auto package = result.Matches().GetAt(0).CatalogPackage();

    deployment::InstallOptions installOptions;

    auto operation = manager.InstallPackageAsync(package, installOptions);
    if (armCancel)
    {
        armCancel([operation]() mutable { operation.Cancel(); });
    }

    struct ProgressState
    {
        std::mutex mutex;
        InstallState lastState = InstallState::Queued;
        int lastPercent = -1;
        uint64_t lastDownloadedBytes = 0;
    };

    auto progressState = std::make_shared<ProgressState>();

    operation.Progress(
        [onProgress, progressState](const auto &, const deployment::InstallProgress &progress)
        {
            if (!onProgress)
            {
                return;
            }

            InstallProgressInfo info{};

            switch (progress.State)
            {
            case deployment::PackageInstallProgressState::Queued:
                info.state = InstallState::Queued;
                info.percent = 0.0;
                break;

            case deployment::PackageInstallProgressState::Downloading:
                info.state = InstallState::Downloading;
                info.percent = progress.DownloadProgress * 100.0;
                info.downloadedBytes = progress.BytesDownloaded;
                info.totalBytes = progress.BytesRequired;
                break;

            case deployment::PackageInstallProgressState::Installing:
                info.state = InstallState::Installing;
                info.percent = progress.InstallationProgress * 100.0;
                break;

            default:
                return;
            }

            info.percent = std::clamp(info.percent, 0.0, 100.0);
            const int roundedPercent = static_cast<int>(std::round(info.percent));

            {
                std::lock_guard lock(progressState->mutex);

                const int stateDiff = static_cast<int>(info.state) - static_cast<int>(progressState->lastState);

                const bool stateAdvanced = stateDiff > 0;
                const bool sameState = stateDiff == 0;

                const bool percentIncreased = roundedPercent > progressState->lastPercent;

                const bool bytesIncreased = info.state == InstallState::Downloading &&
                                            info.downloadedBytes > progressState->lastDownloadedBytes;

                if (!stateAdvanced && !(sameState && (percentIncreased || bytesIncreased)))
                {
                    return;
                }

                progressState->lastState = info.state;
                progressState->lastPercent = roundedPercent;

                if (info.state == InstallState::Downloading)
                {
                    progressState->lastDownloadedBytes =
                        std::max(progressState->lastDownloadedBytes, info.downloadedBytes);
                }
            }

            info.percent = roundedPercent;
            onProgress(info);
        });

    auto installResult = operation.get();
    if (installResult.Status() != deployment::InstallResultStatus::Ok)
    {
        const auto errorCode = installResult.ExtendedErrorCode();

        std::ostringstream message;
        message << "Package installation failed. HRESULT: 0x" << std::hex << static_cast<unsigned int>(errorCode.value);

        throw std::runtime_error(message.str());
    }

    if (onProgress)
    {
        onProgress({InstallState::Completed, 100.0});
    }

    return versionAfterChange(package, packageId);
}

void WingetService::UninstallApp(const std::wstring &packageId, UninstallProgressCallback onProgress) const
{
    uninstallPackage(packageId, std::move(onProgress), {});
}

void WingetService::UninstallAppAsync(const std::wstring &packageId, UninstallProgressCallback onProgress,
                                      UninstallCallback onComplete)
{
    {
        std::lock_guard lock(m_uninstallMutex);
        if (m_uninstallStop || m_activeUninstallId == packageId)
        {
            return;
        }

        for (const UninstallRequest &queued : m_uninstallQueue)
        {
            if (queued.packageId == packageId)
            {
                return;
            }
        }

        m_uninstallQueue.push_back(UninstallRequest{packageId, std::move(onProgress), std::move(onComplete)});
    }

    m_uninstallCv.notify_one();
}

void WingetService::uninstallWorkerMain()
{
    const WorkerApartment apartment;

    for (;;)
    {
        UninstallRequest request;
        {
            std::unique_lock lock(m_uninstallMutex);
            m_uninstallCv.wait(lock, [this] { return m_uninstallStop || !m_uninstallQueue.empty(); });

            if (m_uninstallStop)
            {
                return;
            }

            request = std::move(m_uninstallQueue.front());
            m_uninstallQueue.pop_front();
            m_activeUninstallId = request.packageId;
        }

        struct ClearActive
        {
            WingetService *self;

            ~ClearActive()
            {
                std::lock_guard lock(self->m_uninstallMutex);
                self->m_cancelUninstall = nullptr;
                self->m_activeUninstallId.clear();
            }
        } clearActive{this};

        UninstallCompletion completion;
        try
        {
            uninstallPackage(request.packageId, request.onProgress, [this](std::function<void()> cancel) {
                std::lock_guard lock(m_uninstallMutex);
                if (m_uninstallStop)
                {
                    cancel();
                    return;
                }

                m_cancelUninstall = std::move(cancel);
            });
        }
        catch (const winrt::hresult_error &ex)
        {
            completion.error = winrt::to_string(ex.message());
        }
        catch (const std::exception &ex)
        {
            completion.error = ex.what();
        }

        bool stopped = false;
        {
            std::lock_guard lock(m_uninstallMutex);
            stopped = m_uninstallStop;
        }

        if (stopped)
        {
            return;
        }

        if (request.onComplete)
        {
            request.onComplete(std::move(completion));
        }
    }
}

void WingetService::uninstallPackage(const std::wstring &packageId, UninstallProgressCallback onProgress,
                                     std::function<void(std::function<void()>)> armCancel) const
{
    deployment::PackageManager manager;

    auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
    if (!storeCatalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    deployment::CreateCompositePackageCatalogOptions compositeOptions;
    compositeOptions.Catalogs().Append(storeCatalog);
    compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::RemotePackagesFromAllCatalogs);

    auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
    if (!compositeCatalog)
    {
        throw std::runtime_error("Failed to create composite catalog");
    }

    auto connection = compositeCatalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to composite catalog");
    }

    deployment::FindPackagesOptions options;
    deployment::PackageMatchFilter filter;

    filter.Field(deployment::PackageMatchField::Id);
    filter.Option(deployment::PackageFieldMatchOption::Equals);
    filter.Value(packageId);

    options.Selectors().Append(filter);

    auto result = connection.PackageCatalog().FindPackages(options);
    if (result.Status() != deployment::FindPackagesResultStatus::Ok)
    {
        throw std::runtime_error("Failed to search composite catalog");
    }

    if (result.Matches().Size() == 0)
    {
        throw std::runtime_error("Package not found in composite catalog");
    }

    auto package = result.Matches().GetAt(0).CatalogPackage();
    if (!package.InstalledVersion())
    {
        throw std::runtime_error("Package is not installed");
    }

    deployment::UninstallOptions uninstallOptions;
    auto operation = manager.UninstallPackageAsync(package, uninstallOptions);
    if (armCancel)
    {
        armCancel([operation]() mutable { operation.Cancel(); });
    }

    struct ProgressState
    {
        std::mutex mutex;
        UninstallState lastState = UninstallState::Queued;
        int lastPercent = -1;
    };

    auto progressState = std::make_shared<ProgressState>();

    operation.Progress(
        [onProgress, progressState](const auto &, const deployment::UninstallProgress &progress)
        {
            if (!onProgress)
            {
                return;
            }

            UninstallProgressInfo info{};

            switch (progress.State)
            {
            case deployment::PackageUninstallProgressState::Queued:
                info.state = UninstallState::Queued;
                info.percent = 0.0;
                break;

            case deployment::PackageUninstallProgressState::Uninstalling:
            case deployment::PackageUninstallProgressState::PostUninstall:
                info.state = UninstallState::Uninstalling;
                info.percent = progress.UninstallationProgress * 100.0;
                break;

            default:
                return;
            }

            info.percent = std::clamp(info.percent, 0.0, 100.0);
            const int roundedPercent = static_cast<int>(std::round(info.percent));

            {
                std::lock_guard lock(progressState->mutex);

                const int stateDiff = static_cast<int>(info.state) - static_cast<int>(progressState->lastState);

                const bool stateAdvanced = stateDiff > 0;
                const bool sameState = stateDiff == 0;

                const bool percentIncreased = roundedPercent > progressState->lastPercent;

                if (!stateAdvanced && !(sameState && percentIncreased))
                {
                    return;
                }

                progressState->lastState = info.state;
                progressState->lastPercent = roundedPercent;
            }

            info.percent = roundedPercent;
            onProgress(info);
        });

    auto uninstallResult = operation.get();
    if (uninstallResult.Status() != deployment::UninstallResultStatus::Ok)
    {
        const auto errorCode = uninstallResult.ExtendedErrorCode();

        std::ostringstream message;
        message << "Package uninstall failed. HRESULT: 0x" << std::hex << static_cast<unsigned int>(errorCode.value);

        throw std::runtime_error(message.str());
    }

    if (onProgress)
    {
        onProgress({UninstallState::Completed, 100.0});
    }
}

void WingetService::UpdateApp(const std::wstring &packageId, UpdateProgressCallback onProgress) const
{
    updatePackage(packageId, std::move(onProgress), {});
}

void WingetService::UpdateAppAsync(const std::wstring &packageId, UpdateProgressCallback onProgress,
                                   UpdateCallback onComplete)
{
    {
        std::lock_guard lock(m_updateMutex);
        if (m_updateStop || m_activeUpdateId == packageId)
        {
            return;
        }

        for (const UpdateRequest &queued : m_updateQueue)
        {
            if (queued.packageId == packageId)
            {
                return;
            }
        }

        m_updateQueue.push_back(UpdateRequest{packageId, std::move(onProgress), std::move(onComplete)});
    }

    m_updateCv.notify_one();
}

void WingetService::updateWorkerMain()
{
    const WorkerApartment apartment;

    for (;;)
    {
        UpdateRequest request;
        {
            std::unique_lock lock(m_updateMutex);
            m_updateCv.wait(lock, [this] { return m_updateStop || !m_updateQueue.empty(); });

            if (m_updateStop)
            {
                return;
            }

            request = std::move(m_updateQueue.front());
            m_updateQueue.pop_front();
            m_activeUpdateId = request.packageId;
        }

        struct ClearActive
        {
            WingetService *self;

            ~ClearActive()
            {
                std::lock_guard lock(self->m_updateMutex);
                self->m_cancelUpdate = nullptr;
                self->m_activeUpdateId.clear();
            }
        } clearActive{this};

        UpdateCompletion completion;
        try
        {
            completion.installedVersion = updatePackage(
                request.packageId, request.onProgress, [this](std::function<void()> cancel) {
                    std::lock_guard lock(m_updateMutex);
                    if (m_updateStop)
                    {
                        cancel();
                        return;
                    }

                    m_cancelUpdate = std::move(cancel);
                });
        }
        catch (const winrt::hresult_error &ex)
        {
            completion.error = winrt::to_string(ex.message());
        }
        catch (const std::exception &ex)
        {
            completion.error = ex.what();
        }

        bool stopped = false;
        {
            std::lock_guard lock(m_updateMutex);
            stopped = m_updateStop;
        }

        if (stopped)
        {
            return;
        }

        if (request.onComplete)
        {
            request.onComplete(std::move(completion));
        }
    }
}

std::wstring WingetService::updatePackage(const std::wstring &packageId, UpdateProgressCallback onProgress,
                                          std::function<void(std::function<void()>)> armCancel) const
{
    deployment::PackageManager manager;

    auto storeCatalog = manager.GetPackageCatalogByName(L"msstore");
    if (!storeCatalog)
    {
        throw std::runtime_error("msstore catalog not found");
    }

    deployment::CreateCompositePackageCatalogOptions compositeOptions;
    compositeOptions.Catalogs().Append(storeCatalog);
    compositeOptions.CompositeSearchBehavior(deployment::CompositeSearchBehavior::RemotePackagesFromAllCatalogs);

    auto compositeCatalog = manager.CreateCompositePackageCatalog(compositeOptions);
    if (!compositeCatalog)
    {
        throw std::runtime_error("Failed to create composite catalog");
    }

    auto connection = compositeCatalog.Connect();
    if (connection.Status() != deployment::ConnectResultStatus::Ok)
    {
        throw std::runtime_error("Failed to connect to composite catalog");
    }

    deployment::FindPackagesOptions options;
    deployment::PackageMatchFilter filter;

    filter.Field(deployment::PackageMatchField::Id);
    filter.Option(deployment::PackageFieldMatchOption::Equals);
    filter.Value(packageId);

    options.Selectors().Append(filter);

    auto result = connection.PackageCatalog().FindPackages(options);
    if (result.Status() != deployment::FindPackagesResultStatus::Ok)
    {
        throw std::runtime_error("Failed to search composite catalog");
    }

    if (result.Matches().Size() == 0)
    {
        throw std::runtime_error("Package not found in composite catalog");
    }

    auto package = result.Matches().GetAt(0).CatalogPackage();
    if (!package.InstalledVersion())
    {
        throw std::runtime_error("Package is not installed");
    }

    deployment::InstallOptions updateOptions;
    updateOptions.AllowUpgradeToUnknownVersion(true);

    auto operation = manager.UpgradePackageAsync(package, updateOptions);
    if (armCancel)
    {
        armCancel([operation]() mutable { operation.Cancel(); });
    }

    struct ProgressState
    {
        std::mutex mutex;
        UpdateState lastState = UpdateState::Queued;
        int lastPercent = -1;
        uint64_t lastDownloadedBytes = 0;
    };

    auto progressState = std::make_shared<ProgressState>();

    operation.Progress(
        [onProgress, progressState](const auto &, const deployment::InstallProgress &progress)
        {
            if (!onProgress)
            {
                return;
            }

            UpdateProgressInfo info{};

            switch (progress.State)
            {
            case deployment::PackageInstallProgressState::Queued:
                info.state = UpdateState::Queued;
                info.percent = 0.0;
                break;

            case deployment::PackageInstallProgressState::Downloading:
                info.state = UpdateState::Downloading;
                info.percent = progress.DownloadProgress * 100.0;
                info.downloadedBytes = progress.BytesDownloaded;
                info.totalBytes = progress.BytesRequired;
                break;

            case deployment::PackageInstallProgressState::Installing:
            case deployment::PackageInstallProgressState::PostInstall:
                info.state = UpdateState::Updating;
                info.percent = progress.InstallationProgress * 100.0;
                break;

            default:
                return;
            }

            info.percent = std::clamp(info.percent, 0.0, 100.0);
            const int roundedPercent = static_cast<int>(std::round(info.percent));

            {
                std::lock_guard lock(progressState->mutex);

                const int stateDiff = static_cast<int>(info.state) - static_cast<int>(progressState->lastState);

                const bool stateAdvanced = stateDiff > 0;
                const bool sameState = stateDiff == 0;

                const bool percentIncreased = roundedPercent > progressState->lastPercent;
                const bool bytesIncreased =
                    info.state == UpdateState::Downloading && info.downloadedBytes > progressState->lastDownloadedBytes;

                if (!stateAdvanced && !(sameState && (percentIncreased || bytesIncreased)))
                {
                    return;
                }

                progressState->lastState = info.state;
                progressState->lastPercent = roundedPercent;

                if (info.state == UpdateState::Downloading)
                {
                    progressState->lastDownloadedBytes = info.downloadedBytes;
                }
            }

            info.percent = roundedPercent;
            onProgress(info);
        });

    auto updateResult = operation.get();
    if (updateResult.Status() != deployment::InstallResultStatus::Ok)
    {
        const auto errorCode = updateResult.ExtendedErrorCode();

        if (static_cast<uint32_t>(errorCode.value) == WingetErrors::UpdateNotApplicable)
        {
            if (auto installed = package.InstalledVersion())
            {
                return installed.Version().c_str();
            }

            return {};
        }

        std::ostringstream message;
        message << "Package update failed. HRESULT: 0x" << std::hex << static_cast<unsigned int>(errorCode.value);

        throw std::runtime_error(message.str());
    }

    if (onProgress)
    {
        onProgress({UpdateState::Completed, 100.0});
    }

    return versionAfterChange(package, packageId);
}
