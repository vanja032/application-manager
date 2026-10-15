---
title: WingetPoC
tags:
  - poc
status: current
updated: 2026-10-15
---

# WingetPoC

Console program, `WingetPoC/WingetPoC.cpp`. It exercises [[WingetService]] without the GUI.

## Flow

1. `winrt::init_apartment()`.
2. `SearchApps(L"whatsapp")`.
3. Print the name and id of the first hit.
4. y/n prompts, in order: install, update, uninstall.
5. Progress is printed as state, percent, and, during download, megabytes.

A WinRT error goes to `wcerr` and returns code 1. `std::exception` goes to `cerr`.

The project references ApplicationCore and carries the same NuGet packages: CppWinRT 3.0.260818.1 and WindowsPackageManager.ComInterop 1.29.380.

Debug|Win32, Debug|x64, Release|Win32, and Release|x64 use C++20, the console subsystem, and include path `$(SolutionDir)ApplicationCore\include`. The project does not use a precompiled header.
