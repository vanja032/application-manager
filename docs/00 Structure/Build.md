---
title: Build
tags:
  - structure
  - build
status: current
updated: 2026-10-15
---

# Build

Build the solution, not the `vcxproj` alone. Building the GUI project without the solution can nest intermediates under `ApplicationGUI/ApplicationGUI/`.

Command used for the Debug x64 GUI:

```
MSBuild ApplicationManager.sln /t:ApplicationGUI /p:Configuration=Debug /p:Platform=x64 /p:QtMsBuild="<local QtMsBuild>"
```

`/t:ApplicationGUI` also builds ApplicationCore because of the project reference. Qt MSBuild must find `qt.targets`. On this machine that is the local QtMsBuild directory, and the Qt 6.12.0 msvc2022_64 install is the one the project requests under the name `6.12.0_msvc2022_64`.

Output: `x64/Debug/ApplicationGUI.exe`.

Release|x64 uses the same command with `Configuration=Release`. Output is `x64/Release/ApplicationGUI.exe`. The executable does not carry the Qt DLLs. From the repository root, `windeployqt` from the same Qt kit copies them next to it:

```
<Qt>\6.12.0\msvc2022_64\bin\windeployqt.exe --release --compiler-runtime x64\Release\ApplicationGUI.exe
```

Then start `x64\Release\ApplicationGUI.exe`. The deploy also writes `platforms\qwindows.dll`. That folder has to stay with the executable. If `VCINSTALLDIR` is unset, `--compiler-runtime` skips the Visual C++ runtime. Run the command from a VS 2022 developer prompt, or install the Visual C++ 2022 x64 redistributable on a machine without Visual Studio. Debug uses `--debug` and `x64\Debug\ApplicationGUI.exe`. The root `README.md` has the same steps for someone running the Release build.

ApplicationCore and WingetPoC use C++20 and no precompiled header in Debug and Release, Win32 and x64.

A `.qss` edit alone does not rerun `rcc` if `ApplicationGUI.qrc` did not change. Resources are packed again when the qrc file changes or its timestamp is refreshed.

Qt steps in the GUI build: `uic` for `ui/MainWindow.ui`, `moc` for headers in `include/`, `rcc` for `resources/ApplicationGUI.qrc`.

Related: [[ApplicationGUI project settings]], [[Solution]].
