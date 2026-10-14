---
title: Decision 0002 Microsoft Store only
tags:
  - decisions
  - winget
status: current
updated: 2026-10-14
---

# Decision 0002 Microsoft Store only

## State

Both the backend and the visible GUI work with the Microsoft Store catalog.

## Choice

`WingetService` opens the `msstore` catalog. The GUI source combo offers only Microsoft Store. All sources and winget are not in the combo box. The Updates badge counts only packages whose source is Microsoft Store and whose status is Update available.

The mock catalog still contains winget packages (Visual Studio Code, Firefox, PowerToys, VLC), but the source filter does not show them.

## Consequence

A `whatsapp` search on the initial screen shows six Store cards. The badge shows 2: WhatsApp Business and Spotify.
