---
title: Production goal
tags:
  - goal
  - assessment
status: current
updated: 2026-10-14
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
| Language | C++20 (`/std:c++20`) | Partial. Debug\|x64 sets it. Release on ApplicationCore and WingetPoC does not match. See [[Open questions]]. |
| Target OS | Windows 10 / 11 | Met as the target. Not yet exercised as a finished app on both. |

## Core requirements

### 1. Store-like experience and app tracking

| Requirement | Status |
| --- | --- |
| Experience similar to the Microsoft Store for managing desktop software | Partial. [[ApplicationGUI]] is a visual shell. Data is the [[Mock catalog]], not the live store. |
| Main view with a live list of tracked and installed applications and real-time status (Available, Installing, Installed, Error) | Not met. Cards show Not installed, Installed, Update available, Installing, and Updating from mock data. There is no Error status and no live query. |
| Search, inspect, and install applications from the Microsoft Store source | Partial. The search field and Microsoft Store source exist in the UI. [[WingetService]] can search and install `msstore` packages from [[WingetPoC]]. The GUI does not call it, and card buttons have no handlers. |

### 2. Technology choice

The assessment asks for an independent choice of winget integration aimed at Microsoft Store packages, balancing speed, stability, and a clean C++ architecture. Any C++ GUI framework is allowed.

| Choice | Status |
| --- | --- |
| Backend: in-process Windows Package Manager COM (`Microsoft.Management.Deployment`), catalog `msstore`, wrapped by [[WingetService]] | Met as the selected integration. Proven from the console. Not used by the GUI. See [[Decision 0006 Stack]]. |
| UI: Qt 6.12 Widgets | Met as the selected framework. See [[Decision 0006 Stack]]. |

### 3. UI and asynchronous architecture

| Requirement | Status |
| --- | --- |
| GUI stays responsive. Download, install, and status queries never block the interface | Not met. `InstallApp`, `UninstallApp`, and `UpdateApp` wait on the WinRT operation with `.get()`. Nothing in the GUI runs those calls off the UI thread. |

## Bonus features

These are in scope for the finished application.

| Feature | Status |
| --- | --- |
| Launch an installed Microsoft Store app from the UI | Not met. |
| Uninstall from the application | Partial. [[WingetService]] and [[WingetPoC]] can uninstall. The GUI cannot. |
| Detect available updates and apply them | Partial. [[WingetService]] and [[WingetPoC]] can update. The GUI shows mock update cards only. |

## Deliverables

| Deliverable | Status |
| --- | --- |
| Complete Visual Studio 2022 solution, toolset v143, C++20 | Partial. The solution builds Debug\|x64. Release is not aligned. See [[Build]]. |
| `README.md` with prerequisites, dependencies, build instructions, and a short architectural overview of the backend integration and the UI framework | Not met. [[README outline]] is the planned contents. The root README is not written yet. |

## Rules from the assessment

AI assistance, Visual Studio features, and third-party open-source libraries are allowed.

## How to update this note

When a task moves a row from Not met or Partial to a later state, change that row and add a line to [[Log]]. The row is Met only when the behavior works in the GUI against the live Microsoft Store catalog, with the UI still responsive.
