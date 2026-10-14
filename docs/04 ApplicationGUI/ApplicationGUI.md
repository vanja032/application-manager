---
title: ApplicationGUI
tags:
  - gui
status: current
updated: 2026-10-14
---

# ApplicationGUI

Qt 6.12 Widgets application. The entry point is `src/main.cpp`: Fusion style when it is available, Segoe UI 10 pt, then `MainWindow`.

The project stays away from backend code while a task is interface-only. See [[Decision 0001 UI before backend]].

## Parts

- [[Main window]] — `include/MainWindow.h`, `src/MainWindow.cpp`, `ui/MainWindow.ui`
- [[Application card]] — `AppCard`
- [[Circular progress]] — `CircularProgress`
- [[Icons and theme]] — `UiIcons` and QSS
- [[Mock catalog]] — the data that fills the list
- [[ApplicationGUI project settings]]

`ToggleSwitch` is not its own file. It is a `QWidget` in `MainWindow.cpp`, without `Q_OBJECT`, with a callback. It has to live in the global namespace so moc can see the forward declaration in the header.

Window: title Windows App Manager, initial size about 1180×780, minimum 1000×640. No menu, toolbar, or status bar.
