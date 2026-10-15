---
title: Production goal
tags:
  - goal
  - assessment
status: current
updated: 2026-10-15
---

# Production goal

This is the acceptance bar for a production-ready Windows App Manager. The application is done when every core requirement and every deliverable below is met. Bonus items are part of the product goal for this repository, not optional leftovers.

Source: C++ Technical Assessment, Application Manager (MS Store-like behavior).

## Objective

A C++ Windows desktop application that behaves like the Microsoft Store. The GUI searches, installs, tracks, and manages Microsoft Store applications through the Windows Package Manager (winget) system.

## Technical specifications

| Spec | Requirement | Status |
| --- | --- | --- |
| IDE | Visual Studio 2022 | Met. `ApplicationManager.sln`. |
| Platform toolset | v143 | Met. |
| Language | C++20 (`/std:c++20`) | Met. Debug and Release, Win32 and x64, set `stdcpp20` on ApplicationCore and WingetPoC. ApplicationGUI sets it for Debug\|x64 and Release\|x64. |
| Target OS | Windows 10 / 11 | Met as the target. Not yet exercised as a finished app on both. |

## Core requirements

### 1. Store-like experience and app tracking

| Requirement | Status |
| --- | --- |
| Experience similar to the Microsoft Store for managing desktop software | Met. The GUI searches, installs, updates, uninstalls, and launches Microsoft Store applications, opens a detail page, and offers Pause and Cancel while a download is still queued or downloading. |
| Main view with a live list of tracked and installed applications and real-time status (Available, Installing, Installed, Error) | Met. Discover searches the catalog. Installed lists installed Microsoft Store packages. An operation moves through Queued, Downloading, Installing or Updating, Installed, or Error. |
| Search, inspect, and install applications from the Microsoft Store source | Met. Search and install go through Core. A card click opens a detail page with status, versions, and Launch, Uninstall, Update, or Install. |

### 2. Technology choice

The assessment asks for an independent choice of winget integration aimed at Microsoft Store packages, balancing speed, stability, and a clean C++ architecture. Any C++ GUI framework is allowed.

| Choice | Status |
| --- | --- |
| Backend: in-process Windows Package Manager COM (`Microsoft.Management.Deployment`), catalog `msstore`, wrapped by [[WingetService]] | Met as the selected integration. The GUI uses it for search, install, uninstall, and update. See [[Decision 0006 Stack]]. |
| UI: Qt 6.12 Widgets | Met as the selected framework. See [[Decision 0006 Stack]]. |

### 3. UI and asynchronous architecture

| Requirement | Status |
| --- | --- |
| GUI stays responsive. Download, install, and status queries never block the interface | Met. Search, install, uninstall, update, and launch lookup each run on a Core worker. The window thread only applies the result and activates a resolved package. |

## Bonus features

These are in scope for the finished application.

| Feature | Status |
| --- | --- |
| Launch an installed Microsoft Store app from the UI | Met. Launch sits above Uninstall on an installed card and on the detail page. The worker resolves an `AppUserModelId`, and the window thread activates that package. |
| Uninstall from the application | Met. The GUI uninstalls an installed Microsoft Store package and shows progress or Error. |
| Detect available updates and apply them | Met. Updates lists packages with `IsUpdateAvailable`. The GUI updates that card and shows progress or Error. |

## Deliverables

| Deliverable | Status |
| --- | --- |
| Complete Visual Studio 2022 solution, toolset v143, C++20 | Met. Debug\|x64 and Release\|x64 build. ApplicationCore and WingetPoC also build Win32. The GUI project is x64, and the solution maps its x86 entries to x64. |
| `README.md` with prerequisites, dependencies, build instructions, and a short architectural overview of the backend integration and the UI framework | Met. Root `README.md`, written from [[README outline]]. |

## Rules from the assessment

AI assistance, Visual Studio features, and third-party open-source libraries are allowed.

## How to update this note

When a task moves a row from Not met or Partial to a later state, change that row and add a line to [[Log]]. The row is Met only when the behavior works in the GUI against the live Microsoft Store catalog, with the UI still responsive.
