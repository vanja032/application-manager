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

} // namespace

WingetService::WingetService() : m_worker(&WingetService::workerMain, this)
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
            completion.apps = SearchApps(request.query);
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
        m_pending = SearchRequest{query, std::move(callback)};
    }

    m_cv.notify_one();
}

std::vector<AppInfo> WingetService::SearchApps(const std::wstring &query) const
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
        auto package = match.CatalogPackage();

        AppInfo app;
        app.id = package.Id().c_str();
        app.name = package.Name().c_str();

        apps.push_back(std::move(app));
    }

    return apps;
}

void WingetService::InstallApp(const std::wstring &packageId, ProgressCallback onProgress) const
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
}

void WingetService::UninstallApp(const std::wstring &packageId, UninstallProgressCallback onProgress) const
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
            return;
        }

        std::ostringstream message;
        message << "Package update failed. HRESULT: 0x" << std::hex << static_cast<unsigned int>(errorCode.value);

        throw std::runtime_error(message.str());
    }

    if (onProgress)
    {
        onProgress({UpdateState::Completed, 100.0});
    }
}