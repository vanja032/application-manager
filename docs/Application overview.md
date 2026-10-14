---
title: Application overview
tags:
  - overview
status: current
updated: 2026-10-14
---

# Application overview

Windows App Manager is a C++ Windows desktop application meant to behave like the Microsoft Store. The acceptance bar is [[Production goal]]. The backend catalog today is Microsoft Store (`msstore`), reached through the Windows Package Manager COM API.

Three projects in `ApplicationManager.sln` split that work:

| Project | Role | State |
| --- | --- | --- |
| [[ApplicationCore]] | Static library. Search and package operations. | Search, install, update, and uninstall run against `msstore`. |
| [[WingetPoC]] | Console proof that the service works. | Manual flow for the first `whatsapp` search hit. |
| [[ApplicationGUI]] | Qt Widgets interface. | Search calls `SearchAppsAsync`. Install, update, and uninstall are not connected. |

ApplicationGUI references ApplicationCore. The window calls `SearchAppsAsync` only.

Related: [[Production goal]], [[Solution]], [[Decision 0001 UI before backend]], [[Decision 0002 Microsoft Store only]], [[Decision 0006 Stack]], [[Store search]].
