---
title: Decision 0006 Stack
tags:
  - decisions
  - architecture
status: current
updated: 2026-10-15
---

# Decision 0006 Stack

## State

The assessment in [[Production goal]] allows any winget integration and any C++ GUI framework. It asks for the option that combines speed, stability, and a clean C++ architecture.

## Choice

Backend integration is the Windows Package Manager COM API, namespace `winrt::Microsoft::Management::Deployment`, through [[WingetService]]. The catalog is `msstore`. The process does not shell out to `winget.exe`. The COM API is in-process, typed, and already exercised by [[WingetPoC]].

The UI framework is Qt 6.12 Widgets, Visual Studio 2022, toolset v143, C++20. Layout stays in Qt Designer plus `QWidget` cards. No QML and no Qt Quick.

## Consequence

[[ApplicationCore]] owns the COM calls. [[ApplicationGUI]] calls that library on worker threads and applies results on the window thread. See [[Production goal]] and the root `README.md`.
