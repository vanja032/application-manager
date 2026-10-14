---
title: Icons and theme
tags:
  - gui
status: current
updated: 2026-10-14
---

# Icons and theme

## UiIcons

`include/UiIcons.h`, `src/UiIcons.cpp`. Glyphs: Search, Grid, Refresh, Settings, Moon, Info, Check, Dot, Package, Close.

Search, Grid, Refresh, Settings, Moon, and Info are drawn with the MDL2 font (`segmdl2.ttf`): E721, E71D, E895, E713, E708, E946. Check is a green circle with a white tick. Dot has a diameter of 0.42 of the rectangle. Close is two strokes, drawn at 14 px on the search field. Package is an isometric cube. `windowsLogo` is the four-color Windows mark in the sidebar header.

Pixmaps use the primary screen `devicePixelRatio`.

## Themes

`applyTheme` loads `:/styles.qss` or `:/styles-dark.qss`, sets the palette, and recolors icons and the ring. QSS is applied on `qApp`.

Light page `#F3F5F9`, sidebar `#F7F8FB`, accent `#0078D4`, installed `#107C10`. Dark page `#171B22`, sidebar `#1C212B`, cards `#242A34`, accent `#4DA3FF`, installed `#6CCB5F`.

The combo is flat, at least 38 px tall, with the chevron from `resources/icons/chevron.svg` or `chevron-dark.svg`. Popup items have a height of 32 and padding 6×12.

Related: [[Decision 0003 Visual language]].
