---
title: Folder layout
tags:
  - structure
status: current
updated: 2026-10-14
---

# Folder layout

The same layout applies to [[ApplicationCore]] and [[ApplicationGUI]]. [[WingetPoC]] is still a single `WingetPoC.cpp` in its project root.

```
ApplicationManager/
  ApplicationCore/
    include/          declarations
    src/              implementations
  ApplicationGUI/
    include/          declarations
    src/              implementations and main.cpp
    ui/               Qt Designer
    resources/        QSS, qrc, and icons
  WingetPoC/
  docs/               this vault
  x64/Debug/          build output, not source
```

Declarations are included as `"WingetService.h"` or `"MainWindow.h"`. The compiler finds them because Debug|x64 has `$(ProjectDir)include`. The GUI also has `$(SolutionDir)ApplicationCore\include`, same as WingetPoC.

Headers with `Q_OBJECT` are project items of type `QtMoc`, not also plain `ClInclude`, so they do not appear twice in the solution. `UiIcons.h` has no `Q_OBJECT`, so it is `ClInclude`.

Related: [[Decision 0004 Source layout]], [[ApplicationCore project settings]], [[ApplicationGUI project settings]].
