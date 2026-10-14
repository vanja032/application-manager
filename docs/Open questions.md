---
title: Open questions
tags:
  - backlog
status: current
updated: 2026-10-14
---

# Open questions

Progress against the assessment is tracked in [[Production goal]]. The items below are the gaps that still block that bar.

- The GUI does not call [[WingetService]] yet. Download, install, update, uninstall, and status queries must run off the UI thread. The service currently waits on WinRT with `.get()` and uses `std::wstring`.
- The live list must report Available, Installing, Installed, and Error. The card model has no Error state, and the list is still the [[Mock catalog]].
- There is no action to launch an installed Microsoft Store application from the UI.
- `AppInfo` carries only `id` and `name`. A GUI card also needs source, versions, status, and progress. Those models are not shared yet.
- The `winget` source still exists in the GUI mock catalog, but it is hidden. The backend opens only `msstore`.
- ApplicationCore Release|x64 still expects a precompiled header `pch.h`, which is not in the tree. Debug|x64 is the configuration that actually builds.
- WingetPoC Release|x64 does not set `$(SolutionDir)ApplicationCore\include` or an explicit C++20 standard, unlike Debug|x64.
- ApplicationGUI has only x64 configurations in its project file. The solution x86 entries for that project are mapped to x64.
- Card buttons have no handlers for Install, Update, and Uninstall. There is no Pause or Cancel.
