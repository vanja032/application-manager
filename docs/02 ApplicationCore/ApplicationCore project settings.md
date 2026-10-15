---
title: ApplicationCore project settings
tags:
  - core
  - build
status: current
updated: 2026-10-15
---

# ApplicationCore project settings

`ApplicationCore.vcxproj`. The project type is a static library.

Debug|Win32, Debug|x64, Release|Win32, and Release|x64:

- C++20 (`stdcpp20`)
- no precompiled header (`NotUsing`). There is no `pch.h` in the tree.
- `AdditionalIncludeDirectories`: `$(ProjectDir)include`

Output is `x64/Debug/ApplicationCore.lib` when Debug|x64 is built from the solution. Release|x64 writes `x64/Release/ApplicationCore.lib`. Win32 configurations exist because of the solution x86 platform and use the same compiler settings.

Related: [[Folder layout]].
