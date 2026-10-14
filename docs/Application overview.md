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
| [[ApplicationGUI]] | Qt Widgets interface. | Graphical shell with mock data. It does not call the service. |

ApplicationGUI references ApplicationCore, so the library builds with the GUI project, but no GUI source includes `WingetService.h` or calls its methods.

Related: [[Production goal]], [[Solution]], [[Decision 0001 UI before backend]], [[Decision 0002 Microsoft Store only]], [[Decision 0006 Stack]].
