---
title: Circular progress
tags:
  - gui
status: current
updated: 2026-10-14
---

# Circular progress

`CircularProgress` draws a ring in `paintEvent` with `QPainter`. It is not QML and not a built-in Qt progress bar.

The value is 0–100. The default size is 58 and the pen is 5. Track, value, and text colors change in `setDarkMode`. The percent text is Segoe UI 12 bold.

`drawArc` measures angles in sixteenths of a degree. Zero is at 3 o'clock. The arc starts at 12 o'clock (`90 * 16`) and runs clockwise with a negative span. Qt does not reliably draw a full circle from a 360 degree span, so 100% uses `drawEllipse`.

Related: [[Application card]].
