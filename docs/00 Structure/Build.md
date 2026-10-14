---
title: Build
tags:
  - structure
  - build
status: current
updated: 2026-10-14
---

# Build

Build the solution, not the `vcxproj` alone. Building the GUI project without the solution can nest intermediates under `ApplicationGUI/ApplicationGUI/`.

Command used for the Debug x64 GUI:

```
MSBuild ApplicationManager.sln /t:ApplicationGUI /p:Configuration=Debug /p:Platform=x64 /p:QtMsBuild="<local QtMsBuild>"
```

`/t:ApplicationGUI` also builds ApplicationCore because of the project reference. Qt MSBuild must find `qt.targets`. On this machine that is the local QtMsBuild directory, and the Qt 6.12.0 msvc2022_64 install is the one the project requests under the name `6.12.0_msvc2022_64`.

Output: `x64/Debug/ApplicationGUI.exe`. Running it needs the Qt `bin` directory on `PATH`.

A `.qss` edit alone does not rerun `rcc` if `ApplicationGUI.qrc` did not change. Resources are packed again when the qrc file changes or its timestamp is refreshed.

Qt steps in the GUI build: `uic` for `ui/MainWindow.ui`, `moc` for headers in `include/`, `rcc` for `resources/ApplicationGUI.qrc`.

Related: [[ApplicationGUI project settings]], [[Solution]].
