---
title: Decision 0004 Source layout
tags:
  - decisions
  - structure
status: current
updated: 2026-10-14
---

# Decision 0004 Source layout

## State

ApplicationCore was already split into `include/` and `src/`. ApplicationGUI started with every file in the project root.

## Choice

The GUI follows the same split, plus two Qt folders:

- `include/` for declarations
- `src/` for implementations and `main.cpp`
- `ui/` for the Designer file
- `resources/` for QSS, `ApplicationGUI.qrc`, and SVG icons

Debug and Release GUI configurations get `$(ProjectDir)include`, matching ApplicationCore Debug|x64, and keep `$(SolutionDir)ApplicationCore\include`.

## Consequence

Paths in the `vcxproj` are `include\...`, `src\...`, `ui\MainWindow.ui`, and `resources\...`. Resource paths in code stay `:/styles.qss` and `:/icons/chevron.svg`, because they are relative to the qrc file, which now lives in `resources/`.
