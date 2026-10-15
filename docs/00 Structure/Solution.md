---
title: Solution
tags:
  - structure
status: current
updated: 2026-10-15
---

# Solution

`ApplicationManager.sln` is a Visual Studio 2022 solution, format 12.00. The build tool is MSBuild, toolset v143. ApplicationCore and WingetPoC set C++20 on Debug and Release, Win32 and x64. ApplicationGUI sets C++20 on Debug|x64 and Release|x64. None of the projects use a precompiled header.

Projects, in solution order:

1. [[WingetPoC]], console, GUID `{259E4F71-FD36-43F1-9277-D38F7351026F}`.
2. [[ApplicationCore]], static library, GUID `{5B9E0FF9-25F6-4972-8293-DE2A3C1EFDEC}`.
3. [[ApplicationGUI]], Qt Widgets application, GUID `{2D06C5B9-3576-48AA-B657-125E165BEDAA}`.

WingetPoC and ApplicationGUI reference ApplicationCore.

Solution platforms are Debug|x64, Debug|x86, Release|x64, and Release|x86. Core and PoC have Win32 configurations for x86. The GUI project file has only x64, so the solution x86 entries for the GUI are mapped to x64.

The configuration used so far is Debug|x64. Output goes to `x64/Debug/` at the repository root: `ApplicationCore.lib`, `WingetPoC.exe`, `ApplicationGUI.exe`.

Related: [[Build]], [[Folder layout]].
