---
title: Decision 0002 Microsoft Store only
tags:
  - decisions
  - winget
status: current
updated: 2026-10-15
---

# Decision 0002 Microsoft Store only

## State

Both the backend and the visible GUI work with the Microsoft Store catalog.

## Choice

`WingetService` opens the `msstore` catalog. The GUI offers only Microsoft Store. Search, the featured list, Installed, and Updates all use that catalog. There is no mock catalog.

## Consequence

Discover, Installed, and Updates show Microsoft Store packages only. The window does not list winget packages such as Visual Studio Code or Firefox.
