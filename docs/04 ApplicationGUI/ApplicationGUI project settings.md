---
title: ApplicationGUI project settings
tags:
  - gui
  - build
status: current
updated: 2026-10-14
---

# ApplicationGUI project settings

`ApplicationGUI.vcxproj`. The project type is a Windows application, subsystem Windows. Keyword `QtVS_v304`. Modules: core, gui, widgets. Qt install named in the project: `6.12.0_msvc2022_64`.

Debug|x64 and Release|x64:

- C++20
- `AdditionalIncludeDirectories`: `$(ProjectDir)include;$(SolutionDir)ApplicationCore\include;%(AdditionalIncludeDirectories)`
- reference to ApplicationCore
- `WindowsApp.lib`, required once the GUI calls into the WinRT code inside ApplicationCore

Project items:

- `QtUic`: `ui\MainWindow.ui`
- `QtMoc`: `include\MainWindow.h`, `include\AppCard.h`, `include\CircularProgress.h`
- `QtRcc`: `resources\ApplicationGUI.qrc`
- `ClCompile`: five cpp files in `src\`
- `ClInclude`: `include\UiIcons.h`
- `None`: both QSS files and both chevron SVGs

Filters in `vcxproj.filters` follow the Visual Studio groups Header Files, Source Files, Form Files, and Resource Files. The folders on disk are `include`, `src`, `ui`, and `resources`.

The `qrc` prefix is `/`. Files are listed relative to the qrc, so the application still sees `:/styles.qss`, `:/styles-dark.qss`, `:/icons/chevron.svg`, and `:/icons/chevron-dark.svg`.

Related: [[Build]], [[Decision 0004 Source layout]].
