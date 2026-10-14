---
title: Mock catalog
tags:
  - gui
status: superseded
updated: 2026-10-14
---

# Mock catalog

`mockCatalog()` has been removed from `src/MainWindow.cpp`. The list is filled by `SearchAppsAsync`. The table below is the old sample set and is no longer shown.

The initial search `whatsapp` and the Microsoft Store source show the first six rows.

| Name | Package ID | Source | Status | Note |
| --- | --- | --- | --- | --- |
| WhatsApp | 9NKSQGP7F2NH | Microsoft Store | Installed | |
| WhatsApp Business | 9PG2B0DLY5HT | Microsoft Store | Update available | available 2.2639.100.0 |
| WhatsApp Desktop | 5319275A.WhatsAppDesktop | Microsoft Store | Installing | 68%, 325/520 MB |
| WhatsApp Web | 9NBLGGH4M8R0 | Microsoft Store | Not installed | |
| WhatsApp Beta | 9NKSQGP7F2NH | Microsoft Store | Not installed | |
| WhatsApp Messenger | 9NKSQGP7F2NH | Microsoft Store | Not installed | |
| Visual Studio Code | Microsoft.VisualStudioCode | winget | Update available | 1.95.3 to 1.96.2, hidden |
| Spotify | 9NCBCSZSJRSB | Microsoft Store | Update available | available 1.2.70.0 |
| Mozilla Firefox | Mozilla.Firefox | winget | Installed | 133.0.3, hidden |
| PowerToys | Microsoft.PowerToys | winget | Updating | 42%, 80/190 MB, hidden |
| VLC media player | VideoLAN.VLC | winget | Not installed | hidden |

The Updates badge counts Store packages with Update available: WhatsApp Business and Spotify, so 2. Spotify is included in that count even when the current search does not show its card.

Related: [[Decision 0002 Microsoft Store only]].
