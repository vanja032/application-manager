---
title: ApplicationCore project settings
tags:
  - core
  - build
status: current
updated: 2026-10-14
---

# ApplicationCore project settings

`ApplicationCore.vcxproj`. The project type is a static library.

Debug|x64, the configuration in use:

- C++20 (`stdcpp20`)
- no precompiled header (`NotUsing`)
- `AdditionalIncludeDirectories`: `$(ProjectDir)include`
- output `x64/Debug/ApplicationCore.lib` when built from the solution

Release|x64 still has `PrecompiledHeader` set to Use and file `pch.h`. That file is not among the sources. Recorded in [[Open questions]].

Win32 configurations exist because of the solution x86 platform and are not aligned with Debug|x64.

Related: [[Folder layout]].
