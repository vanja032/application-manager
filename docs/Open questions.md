---
title: Open questions
tags:
  - backlog
status: current
updated: 2026-10-14
---

# Open questions

Progress against the assessment is tracked in [[Production goal]]. The items below are the gaps that still block that bar.

- Update and uninstall still wait on WinRT with `.get()` and are not wired to the buttons. Install from a search card calls `InstallAppAsync`.
- The live list must report Available, Installing, Installed, and Error for tracked apps. Search results now show catalog install state, and an install reports Queued, Downloading, Installing, Installed, or Error. There is still no list of every installed app.
- There is no action to launch an installed Microsoft Store application from the UI.
- `AppInfo` carries only `id` and `name`. A GUI card also needs source, versions, status, and progress. Those models are not shared yet.
- The GUI no longer ships a mock catalog. Search results come from `msstore` only.
- ApplicationCore Release|x64 still expects a precompiled header `pch.h`, which is not in the tree. Debug|x64 is the configuration that actually builds.
- WingetPoC Release|x64 does not set `$(SolutionDir)ApplicationCore\include` or an explicit C++20 standard, unlike Debug|x64.
- ApplicationGUI has only x64 configurations in its project file. The solution x86 entries for that project are mapped to x64.
- Card buttons have no handlers for Install, Update, and Uninstall. There is no Pause or Cancel.
- Store search drops very common English words, so `what` returns nothing while `wha` and `whats` find WhatsApp. Matching the typed characters, explaining that empty result, and searching installed apps locally are recorded in [[Store search]] as later advantages. They are not assessment blockers.
