---
title: AppInfo
tags:
  - core
status: current
updated: 2026-10-14
---

# AppInfo

`include/AppInfo.h`.

```cpp
struct AppInfo
{
    std::wstring id;
    std::wstring name;
    bool installed = false;
    std::wstring installedVersion;
};
```

Search fills `installed` from the catalog package `InstalledVersion`. A null version means the package is not installed. Progress during install is not stored here. The GUI keeps that on the card.
