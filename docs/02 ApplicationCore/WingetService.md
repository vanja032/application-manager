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

- `SearchApps(query)` returns `std::vector<AppInfo>` and blocks until the search finishes. [[WingetPoC]] still uses this.
- `SearchAppsAsync(query, callback)` returns immediately. A Core-owned worker thread calls `winrt::init_apartment` and runs `SearchApps`. The callback receives a `SearchCompletion` with `apps` or `error`. The worker is joined in the destructor. A newer request replaces one that has not started. Install, update, and uninstall are unchanged.
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

`PackageManager::GetPackageCatalogByName(L"msstore")`, then `Connect`. The name filter uses ContainsCaseInsensitive and is added to `Filters`, with `ResultLimit` 10. The query is sent as typed. The catalog drops very common English words before lookup, so a query such as `what` can return nothing even when a package name contains those letters. See [[Store search]]. `Selectors` would resolve a single package, which is why search previously returned one hit. Each match becomes an `AppInfo` with the catalog package `Id` and `Name`.

If the catalog is missing, the connection is not Ok, or the search is not Ok, the method throws `std::runtime_error`.

## Install and uninstall

The package is looked up by id in the same `msstore` catalog. An empty result is the error "Package not found". Install is `InstallPackageAsync`. Uninstall is `UninstallPackageAsync`. After `.get()`, the status must be Ok, otherwise the method throws the HRESULT in the message.

## Update

Update builds a composite catalog from `msstore`, with behavior `RemotePackagesFromAllCatalogs`, so the installed version is visible. The package is looked up by id with option Equals. If there is no `InstalledVersion`, it throws "Package is not installed".

`UpgradePackageAsync` receives `AllowUpgradeToUnknownVersion(true)`. If the result is not Ok and the extended code is `WingetErrors::UpdateNotApplicable` (`0x8A15002B`), the method returns quietly. Other failures throw the HRESULT. `UpgradeVersionNotNewer` and `UpgradeVersionUnknown` are defined, but this path does not handle them separately.

Related: [[AppInfo]], [[WingetErrors]], [[WingetPoC]], [[Store search]].
