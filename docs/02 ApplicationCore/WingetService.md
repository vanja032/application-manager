---
title: WingetService
tags:
  - core
status: current
updated: 2026-10-14
---

# WingetService

`include/WingetService.h`, implementation `src/WingetService.cpp`.

## Public surface

- `SearchApps(query)` returns `std::vector<AppInfo>`.
- `InstallApp(packageId, onProgress)`
- `UninstallApp(packageId, onProgress)`
- `UpdateApp(packageId, onProgress)`

The callback is optional. Without it, the operation still waits until it finishes.

## Progress states

Install: Queued, Downloading, Installing, Completed. During download the callback carries percent and byte counts.

Uninstall: Queued, Uninstalling, Completed. No byte counts.

Update: Queued, Downloading, Updating, Completed. WinRT states Installing and PostInstall map to Updating.

The callback is not invoked on every WinRT tick. It fires only when the state advances, or when the percent or downloaded bytes increase inside the same state. Percent is rounded to an integer.

## Search

`PackageManager::GetPackageCatalogByName(L"msstore")`, then `Connect`. The filter field is Name, option ContainsCaseInsensitive. Each match becomes an `AppInfo` with the catalog package `Id` and `Name`.

If the catalog is missing, the connection is not Ok, or the search is not Ok, the method throws `std::runtime_error`.

## Install and uninstall

The package is looked up by id in the same `msstore` catalog. An empty result is the error "Package not found". Install is `InstallPackageAsync`. Uninstall is `UninstallPackageAsync`. After `.get()`, the status must be Ok, otherwise the method throws the HRESULT in the message.

## Update

Update builds a composite catalog from `msstore`, with behavior `RemotePackagesFromAllCatalogs`, so the installed version is visible. The package is looked up by id with option Equals. If there is no `InstalledVersion`, it throws "Package is not installed".

`UpgradePackageAsync` receives `AllowUpgradeToUnknownVersion(true)`. If the result is not Ok and the extended code is `WingetErrors::UpdateNotApplicable` (`0x8A15002B`), the method returns quietly. Other failures throw the HRESULT. `UpgradeVersionNotNewer` and `UpgradeVersionUnknown` are defined, but this path does not handle them separately.

Related: [[AppInfo]], [[WingetErrors]], [[WingetPoC]].
