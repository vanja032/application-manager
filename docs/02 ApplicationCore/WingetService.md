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
- `SearchAppsAsync(query, callback)` returns immediately. A Core-owned worker thread calls `winrt::init_apartment` and runs `SearchApps`. The callback receives a `SearchCompletion` with `apps` or `error`. The worker is joined in the destructor. A newer request replaces one that has not started.
- `InstallApp(packageId, onProgress)` blocks until install finishes. [[WingetPoC]] still uses this.
- `InstallAppAsync(packageId, onProgress, onComplete)` returns immediately. A second Core worker, separate from search, runs installs one at a time. Progress uses the same `InstallProgressInfo` values as the synchronous method. `onComplete` receives an `InstallCompletion` whose `error` is empty on success and whose `installedVersion` is the version just installed. A second request for a package that is already queued or running is ignored. Closing the service cancels the active WinRT operation and joins the worker.
- `UninstallApp(packageId, onProgress)` blocks until uninstall finishes. [[WingetPoC]] still uses this.
- `UninstallAppAsync(packageId, onProgress, onComplete)` returns immediately. A third Core worker runs uninstalls one at a time. Progress uses `UninstallProgressInfo`. `onComplete` receives an `UninstallCompletion` whose `error` is empty on success. A second request for a package that is already queued or running is ignored. Closing the service cancels the active operation and joins the worker.
- `UpdateApp(packageId, onProgress)` blocks until update finishes. [[WingetPoC]] still uses this. `UpdateNotApplicable` returns without an error.
- `UpdateAppAsync(packageId, onProgress, onComplete)` returns immediately. A fourth Core worker runs updates one at a time. Progress uses `UpdateProgressInfo`. `onComplete` receives an `UpdateCompletion` whose `error` is empty on success, including when the update is not applicable, and whose `installedVersion` is the version now installed. A second request for a package that is already queued or running is ignored. Closing the service cancels the active operation and joins the worker.

The callback is optional. Without it, the operation still waits until it finishes.

## Progress states

Install: Queued, Downloading, Installing, Completed. During download the callback carries percent and byte counts.

Uninstall: Queued, Uninstalling, Completed. No byte counts.

Update: Queued, Downloading, Updating, Completed. WinRT states Installing and PostInstall map to Updating.

The callback is not invoked on every WinRT tick. It fires only when the state advances, or when the percent or downloaded bytes increase inside the same state. Percent is rounded to an integer.

## Search

Search builds a composite catalog from `msstore`, with behavior `RemotePackagesFromAllCatalogs`, then `Connect`. The remote `msstore` catalog alone does not fill `InstalledVersion`, so a package that is already installed looked not installed. The name filter uses ContainsCaseInsensitive and is added to `Filters`, with `ResultLimit` 10. The query is sent as typed. The catalog drops very common English words before lookup, so a query such as `what` can return nothing even when a package name contains those letters. See [[Store search]]. `Selectors` would resolve a single package, which is why search previously returned one hit. Each match becomes an `AppInfo` with the catalog package `Id`, `Name`, and `InstalledVersion` when the composite catalog has one.

If the catalog is missing, the connection is not Ok, or the search is not Ok, the method throws `std::runtime_error`.

## Install and uninstall

The package is looked up by id in the remote `msstore` catalog. An empty result is the error "Package not found". Install is `InstallPackageAsync`. After `.get()`, the status must be Ok, otherwise the method throws the HRESULT in the message. A failed version lookup does not turn that success into an error. The installed version is then read from the composite catalog, the same source search uses, because the remote package leaves `InstalledVersion` empty. If that lookup is empty, the fallback is `DefaultInstallVersion` on the package that was installed. The async install path uses that same body. Its worker catches the exception and puts the text in `InstallCompletion.error`, and copies the returned version into `installedVersion`. Search reads `InstalledVersion`, `IsUpdateAvailable`, and `DefaultInstallVersion` on each composite-catalog match.

Uninstall uses a composite catalog so the installed package is visible. It requires `InstalledVersion`. Uninstall is `UninstallPackageAsync`. The async path uses that same body and reports failure in `UninstallCompletion.error`. WinRT states Uninstalling and PostUninstall both map to Uninstalling.

## Update

Update builds a composite catalog from `msstore`, with behavior `RemotePackagesFromAllCatalogs`, so the installed version is visible. The package is looked up by id with option Equals. If there is no `InstalledVersion`, it throws "Package is not installed".

`UpgradePackageAsync` receives `AllowUpgradeToUnknownVersion(true)`. If the result is not Ok and the extended code is `WingetErrors::UpdateNotApplicable` (`0x8A15002B`), the method returns the current `InstalledVersion` without throwing. Other failures throw the HRESULT. After Ok, the version is read again from the composite catalog, with `DefaultInstallVersion` as the fallback. `UpgradeVersionNotNewer` and `UpgradeVersionUnknown` are defined, but this path does not handle them separately.

Related: [[AppInfo]], [[WingetErrors]], [[WingetPoC]], [[Store search]].
