---
title: WingetErrors
tags:
  - core
status: current
updated: 2026-10-14
---

# WingetErrors

`include/WingetErrors.h`. Constant HRESULT codes the service recognizes during update:

| Name | Value | Use today |
| --- | --- | --- |
| `UpdateNotApplicable` | `0x8A15002B` | `UpdateApp` returns quietly |
| `UpgradeVersionNotNewer` | `0x8A15004F` | defined, not handled separately |
| `UpgradeVersionUnknown` | `0x8A150050` | defined, not handled separately |

Related: [[WingetService]].
