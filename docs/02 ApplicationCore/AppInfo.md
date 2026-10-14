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

Search fills `installed` from `InstalledVersion` on the composite `msstore` catalog. The remote catalog alone leaves that property empty, so an installed package would look available. A null version means the package is not installed. Progress during install is not stored here. The GUI keeps that on the card.
