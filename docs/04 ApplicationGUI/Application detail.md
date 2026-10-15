---
title: Application detail
tags:
  - gui
status: current
updated: 2026-10-15
---

# Application detail

`AppDetailPage` is a `QWidget` added to the main `QStackedWidget` from `MainWindow`. It is not a form in `MainWindow.ui`.

Back is a secondary button, 112×36, on the left. The header is a 64 px package icon, the application name, the package id, and the source. The info card is 520 to 720 px wide, in a horizontal layout with a stretch, so wrapped lines keep their height.

The card lists status, the installed version when the package is not in the Not installed state, the available version when that string is set, the error text after a failed install, update, or uninstall, and progress while an operation is running.

Actions use the same 112×36 primary and secondary buttons as the card, stacked on the left:

- Update available, or an update Error: Update, Launch, Uninstall.
- Installed, or an uninstall Error: Launch, Uninstall.
- Not installed, or an install Error: Install.
- Queued or downloading, for install or update: Pause and Cancel.
- Installing, updating, or uninstalling: no action buttons. The card shows the phase and percent.
- Paused: Resume and Cancel. Progress stays at the last reported percent.

Launch failure is text on this page. It does not change the card to Error.

Related: [[Main window]], [[Application card]], [[Decision 0007 Pause]].
