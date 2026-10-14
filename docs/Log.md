---
title: Log
tags:
  - log
status: current
updated: 2026-10-14
---

# Log

- 2026-10-14: Vault opened. Recorded the solution, ApplicationCore, WingetPoC, ApplicationGUI with its mock UI, the decisions so far, and the open questions.
- 2026-10-14: Vault notes, file names, and folder names moved to English. Source, UI strings, and styles were already English.
- 2026-10-14: Recorded the assessment as [[Production goal]], with current status for each requirement. Stack choice recorded as [[Decision 0006 Stack]].
- 2026-10-14: Search from the GUI uses `SearchAppsAsync` on a Core worker thread. The synchronous `SearchApps` remains for [[WingetPoC]]. Cards show name and package ID only.
- 2026-10-14: Combo popups open below the field. The selected item no longer shifts the list up over the closed combo.
- 2026-10-14: Search uses `Filters` and returns up to 10 matches. The Windows logo tiles are square.
- 2026-10-14: Recorded how `msstore` drops common words, and the search advantages still open versus that index. See [[Store search]].
- 2026-10-14: All, Installed, and Not installed stay visible and do not filter search cards until install status exists.
- 2026-10-14: Install from the GUI calls `InstallAppAsync` on its own Core worker. The card shows Queued, Downloading, Installing, Installed, or Error.
