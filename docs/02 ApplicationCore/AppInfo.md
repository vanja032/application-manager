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
};
```

This is the only model the backend returns from search. It has no source, version, install status, or progress. The GUI model `AppCardModel` is wider and lives only in [[ApplicationGUI]]. Joining the two models is still open.
