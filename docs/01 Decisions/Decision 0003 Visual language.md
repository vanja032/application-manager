---
title: Decision 0003 Visual language
tags:
  - decisions
  - gui
status: current
updated: 2026-10-14
---

# Decision 0003 Visual language

## State

The light theme is the default. The dark theme is switched on from the sidebar.

## Choice

Controls, cards, navigation, dropdowns, the badge, and the scrollbar use `border-radius: 0`. A circle remains only where the shape carries meaning: the progress ring, the dark-mode switch, the green check, and the status dot.

Sidebar: 16 px icon, 10 px gap to the text, 12 px left margin, on every item including Dark mode and About.

Card status: a 16 px icon sits beside the first line, not in the vertical center of the whole block. Version lines sit under the title, indented by the icon width.

Search is a `QFrame`, not the built-in `QLineEdit` clear button. The magnifier and the X sit inset from the field edges. The X uses a pointer cursor.

The list scrollbar is 6 px, with no top or bottom gap, and only inside `QScrollArea#appScrollArea`.

## Where it lives

`resources/styles.qss`, `resources/styles-dark.qss`, `src/MainWindow.cpp`, `src/AppCard.cpp`.
