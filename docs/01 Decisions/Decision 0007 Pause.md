---
title: Decision 0007 Pause
tags:
  - decisions
status: current
updated: 2026-10-15
---

# Decision 0007 Pause

## State

The Microsoft Store shows Pause and Cancel while an application is downloading. [[WingetService]] installs and updates through `InstallPackageAsync` and `UpgradePackageAsync`.

## Choice

Cancel calls `Cancel` on the active async operation, or removes the package from the Core queue when it has not started. A cancelled install returns the card to Not installed. A cancelled update returns it to Update available.

Pause uses that same cancel. The card stays on Paused, with the last percent, and Resume starts the install or update again. The COM contract has no pause or resume method, and `PackageInstallProgressState` has no paused value. Once the state is Installing or Updating, Pause and Cancel are hidden, because cancellation does not stop the installer.

## Consequence

A resumed download can start again from the beginning. The partial download is not held open inside this process.

`msstore` usually never enters Downloading. The operation goes from Queued to Installing with `DownloadProgress` already at 1, because the Store downloads the package and winget does not. Pause and Cancel are therefore often absent for a Store install or update. Recorded in [[WingetService]]. No GUI change for that progress shape yet.

Related: [[WingetService]], [[Application card]], [[Production goal]].
