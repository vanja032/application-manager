// WingetService.cpp

#include "WingetService.h"

#include <winrt/Microsoft.Management.Deployment.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>

namespace deployment = winrt::Microsoft::Management::Deployment;

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

    options.Selectors().Append(filter);

    auto result = connection.PackageCatalog().FindPackages(options);

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