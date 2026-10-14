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

`AppAvailability`: NotInstalled, Installed, UpdateAvailable, Installing, Updating.

## Layout

Three zones: a 48 px package icon, identity (stretches), a status panel fixed at 280, and an action panel of width 112.

Status:

- Installing and Updating: [[Circular progress]] plus the title.
- Everything else: a row with a 16 px icon and the title, then details indented by 24 px. The check and the update icon are the same size and aligned with the first line. The Not installed dot sits vertically with that text.

Action: primary and secondary buttons, 112×36. There are no click handlers. Installing has no Cancel.

Related: [[Mock catalog]].
