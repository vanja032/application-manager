---
title: Main window
tags:
  - gui
status: current
updated: 2026-10-14
---

# Main window

`MainWindow` keeps the static shell from `ui/MainWindow.ui` and the dynamic cards that code adds to the list.

## Sidebar

Width 232. Items: Discover (selected at start), Installed, Updates, Settings, a stretch, Dark mode, About. Discover, Installed, Updates, Settings, and About are a `QButtonGroup`.

Text and icon are not `QPushButton::setIcon`. Each button gets an inner horizontal layout: a `QLabel` for the 16 px icon and a `QLabel` for the text, spacing 10 px. The labels ignore the mouse. Dark mode and About then share the same gap as the other items.

Updates carries a badge, a child of the button, 22×22, transparent to the mouse. The number is the count of Microsoft Store cards with status Update available. Position is set in `eventFilter` and `showEvent`.

## Content

Search row: the field and a Search button, 104×40. The field is `QFrame#searchFrame` (margins 16 and 14, spacing 10, height 40) with an icon, a plain `QLineEdit`, and `QToolButton#searchClearButton`. The built-in clear button is disabled. The X is visible when the text is not empty, uses `PointingHandCursor`, and clears the field. Focus on the frame is the dynamic property `focused`, so QSS changes the border. The initial text is `whatsapp`, and the window starts that search on launch. Search and Enter call `SearchAppsAsync`. Results are applied with `QMetaObject::invokeMethod` and `Qt::QueuedConnection`. A generation counter drops a result when a newer search has already started. While the search runs, the list shows "Searching...". Failure shows the error text. No matches show "No applications match your search."

Beside search is the Source column. The combo has only Microsoft Store. Source and Sort by use `AnchoredComboBox`, so the list opens under the field. The selected row stays in its place in the list and does not cover the closed field.

Filters: All, Installed, Not installed stay on screen. They do not change the search list yet, because a search card has no install status. Sort: Relevance (result order) and Name (`localeAwareCompare`).

`QStackedWidget` has three pages: the list, Settings, and About. The list is `QScrollArea#appScrollArea`. The horizontal scrollbar is off. The vertical scrollbar is 6 px, margin 0, with the thumb inset 1 px on each side. The list content bottom margin is 0 and the right margin is 2 px, so the cards stay wider.

An empty list shows a label instead of the scroll area.

The Settings text says the catalog uses the Microsoft Store and that search matches the application name and package ID.

## Filtering

A search card has no install status. On Discover it stays visible for All, Installed, and Not installed. Those chips are not applied until a card has a real status. The Installed and Updates sections still hide it, because those states are not in `AppInfo` yet.

Sections: Discover shows the current search. Installed and Updates show the empty state until status exists. Settings and About switch the stack page.

Refreshing the list hides non-matches, sorts the visible cards, and restores the scroll position.

Related: [[Application card]], [[Mock catalog]], [[Decision 0003 Visual language]].
