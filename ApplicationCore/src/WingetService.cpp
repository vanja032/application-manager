// WingetService.cpp

#include "WingetService.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.Management.Deployment.h>

//#include <stdexcept>

namespace deployment = winrt::Microsoft::Management::Deployment;

std::vector<AppInfo> WingetService::SearchApps(const std::wstring &query) const
{
    deployment::PackageManager manager;
    
    for(const auto& catalog : manager.GetPackageCatalogs())
    {
        if (catalog.Info().Name() != L"msstore")
        {
            continue;
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

    throw std::runtime_error("msstore catalog not found");
}