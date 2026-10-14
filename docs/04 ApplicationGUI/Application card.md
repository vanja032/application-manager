---
title: Application card
tags:
  - gui
status: current
updated: 2026-10-14
---

# Application card

`AppCard` is a `QWidget` with `Q_OBJECT`. `paintEvent` draws `PE_Widget` so the QSS class selector works. Hover is enabled through `WA_Hover`.

## Model

`AppCardModel`: name, package ID, source, availability, installed version, available version, percent, downloaded MB, total MB, and `catalogIndex` for the Relevance sort.

`AppAvailability`: NotInstalled, Installed, UpdateAvailable, Queued, Downloading, Installing, Updating, UninstallQueued, Uninstalling, Error. `errorText` is the failure message. `retryUninstall` chooses an Uninstall button after a failed removal.

## Layout

Three zones: a 48 px package icon, identity (stretches), a status panel fixed at 280, and an action panel of width 112.

Status:

- Queued, Downloading, Installing, Updating, UninstallQueued, and Uninstalling: [[Circular progress]] plus the phase title. Downloading also shows megabytes when the total is known.
- Error: a red dot, the title Error, and the message wrapped in the status column.
- Everything else: a row with a 16 px icon and the title, then details indented by 24 px. The check and the update icon are the same size and aligned with the first line. The Not installed dot sits vertically with that text.

Action: primary and secondary buttons, 112×36. Not installed and an install Error emit `installRequested`. Installed, and an uninstall Error, emit `uninstallRequested`. Update available still shows Update and Uninstall without handlers. There is no Cancel while a package is queued, downloading, installing, or uninstalling.

Related: [[Mock catalog]].
