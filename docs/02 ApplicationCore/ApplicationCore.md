---
title: ApplicationCore
tags:
  - core
status: current
updated: 2026-10-14
---

# ApplicationCore

Static library. The only place that talks to the Windows Package Manager COM API. The GUI and the console call this layer, not WinRT types.

## Sources

- `include/AppInfo.h` — [[AppInfo]]
- `include/WingetErrors.h` — [[WingetErrors]]
- `include/WingetService.h` and `src/WingetService.cpp` — [[WingetService]]
- `packages.config` — NuGet packages below

## Dependencies

- `Microsoft.Windows.CppWinRT` 3.0.260818.1
- `Microsoft.WindowsPackageManager.ComInterop` 1.29.380

The WinRT namespace in code is `winrt::Microsoft::Management::Deployment`.

## What the library does today

`WingetService` can search packages by name and install, update, or remove a package by id. Everything goes through the `msstore` catalog. Progress comes back through a `std::function` callback. Calls wait for the async operation with `.get()`.

Public methods are PascalCase (`SearchApps`, `InstallApp`). That is the convention of this project. The GUI uses Qt camelCase.

Related: [[ApplicationCore project settings]], [[Decision 0002 Microsoft Store only]], [[Store search]].
