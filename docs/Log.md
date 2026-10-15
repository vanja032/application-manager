---
title: Log
tags:
  - log
status: current
updated: 2026-10-15
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
- 2026-10-14: Uninstall from the GUI calls `UninstallAppAsync` on its own Core worker. The card shows Queued, Uninstalling, Not installed, or Error.
- 2026-10-14: Search uses a composite `msstore` catalog so `InstalledVersion` is set for packages that are already installed.
- 2026-10-14: Update from the GUI calls `UpdateAppAsync` on its own Core worker. Search uses `IsUpdateAvailable` to show Update available.
- 2026-10-14: After install or update, the card version comes from a composite-catalog lookup in the sync install and update path. The remote install package does not fill `InstalledVersion`.
- 2026-10-14: Discover filters search results with All, Installed, and Not installed. Installed and Updates load their own Microsoft Store lists and hide those filters.
- 2026-10-15: Settings text wraps inside the card. About names Vanja Sretenovic and links to vanjasretenovic.com.
- 2026-10-15: The search field starts empty. Discover opens with up to 10 featured Microsoft Store applications.
- 2026-10-15: A card click opens an application page with Back, status, versions, and Launch, Uninstall, Update, or Install. Launch starts the installed Store application.
- 2026-10-15: Launch uses `IApplicationActivationManager` on the window thread. `AppListEntry.LaunchAsync` from the search worker does not start the package.
- 2026-10-15: Install and update can be paused or cancelled from the card and the application page. Pause cancels the active operation and Resume starts it again. See [[Decision 0007 Pause]].
- 2026-10-15: Pause and Cancel hide once an install or update reaches Installing or Updating, because cancellation no longer stops the installer.
- 2026-10-15: Recorded that `msstore` install and update progress skips Downloading and reports `DownloadProgress` as 1 while the state is Installing. Pause and Cancel therefore often do not appear. No code change.
- 2026-10-15: ApplicationCore and WingetPoC use C++20 and the same include paths on Debug and Release, Win32 and x64. Precompiled headers are off. There is no `pch.h`.
- 2026-10-15: Root `README.md` written from [[README outline]]. It covers prerequisites, the Debug and Release x64 build, the winget and Qt split, and the known Store limits.
- 2026-10-15: Release `ApplicationGUI.exe` is started from `x64/Release` after `windeployqt` copies the Qt DLLs beside it. The root README has the full steps, including the Visual C++ runtime when `VCINSTALLDIR` is unset.
- 2026-10-15: Decision 0002 no longer describes a mock catalog. Discover, Installed, and Updates use `msstore` only.
- 2026-10-15: Root README shows a Discover screenshot and links the author to github.com/vanja032 and vanjasretenovic.com.
