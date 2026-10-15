---
title: Open questions
tags:
  - backlog
status: current
updated: 2026-10-15
---

# Open questions

Progress against the assessment is tracked in [[Production goal]]. The notes below describe current behavior and limits. They do not block that bar.

- Install calls `InstallAppAsync`. Uninstall calls `UninstallAppAsync`. Update calls `UpdateAppAsync`. Installed opens `ListInstalledAppsAsync`. Updates opens `ListUpdatesAsync`. Launch calls `LaunchAppAsync`.
- The live list must report Available, Installing, Installed, and Error for tracked apps. Search results and the Installed section show catalog install state, and an install reports Queued, Downloading, Installing, Installed, or Error.
- `AppInfo` carries id, name, install state, and whether an update is available. Source is still not on that model. The card keeps install, uninstall, and update progress locally.
- The GUI no longer ships a mock catalog. Search results come from `msstore` only.
- ApplicationGUI has only x64 configurations in its project file. The solution x86 entries for that project are mapped to x64.
- Store search drops very common English words, so `what` returns nothing while `wha` and `whats` find WhatsApp. Matching the typed characters, explaining that empty result, and searching installed apps locally are recorded in [[Store search]] as later advantages. They are not assessment blockers.
