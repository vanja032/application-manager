---
title: Decision 0001 UI before backend
tags:
  - decisions
  - gui
status: current
updated: 2026-10-14
---

# Decision 0001 UI before backend

## State

The graphical shell exists and uses mock data. [[WingetService]] is not called from the GUI.

## Choice

[[ApplicationGUI]] was built as appearance only. ApplicationCore and WingetPoC stay unchanged during that work. No QML, Qt Quick, CMake, or another UI framework. No Pause or Cancel actions.

## Consequence

The screen can be judged before the service is wired in. Card buttons have no handlers. Connecting the two remains open in [[Open questions]].
