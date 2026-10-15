---
title: Main window
tags:
  - gui
status: current
updated: 2026-10-15
---

# Main window

`MainWindow` keeps the static shell from `ui/MainWindow.ui` and the dynamic cards that code adds to the list.

## Sidebar

Width 232. Items: Discover (selected at start), Installed, Updates, Settings, a stretch, Dark mode, About. Discover, Installed, Updates, Settings, and About are a `QButtonGroup`.

Text and icon are not `QPushButton::setIcon`. Each button gets an inner horizontal layout: a `QLabel` for the 16 px icon and a `QLabel` for the text, spacing 10 px. The labels ignore the mouse. Dark mode and About then share the same gap as the other items.

Updates carries a badge, a child of the button, 22×22, transparent to the mouse. The number is the count of Microsoft Store cards with status Update available. Position is set in `eventFilter` and `showEvent`.

## Content

Search row: the field and a Search button, 104×40. The field is `QFrame#searchFrame` (margins 16 and 14, spacing 10, height 40) with an icon, a plain `QLineEdit`, and `QToolButton#searchClearButton`. The built-in clear button is disabled. The X is visible when the text is not empty, uses `PointingHandCursor`, and clears the field. Focus on the frame is the dynamic property `focused`, so QSS changes the border. The field starts empty. Discover then calls `ListFeaturedAppsAsync` and shows up to 10 well-known Microsoft Store applications. Search and Enter call `SearchAppsAsync`. An empty search returns to that featured list. Results are applied with `QMetaObject::invokeMethod` and `Qt::QueuedConnection`. A generation counter drops a result when a newer search has already started. While a search runs, the list shows "Searching...". The featured load shows "Loading featured applications...". Failure shows the error text. No search matches show "No applications match your search."

Each result card has a status. `InstalledVersion` from the composite catalog chooses Installed or Not installed. `IsUpdateAvailable` chooses Update available. Install calls `InstallAppAsync` and does not block the window. The card then shows Queued, Downloading with megabytes, Installing, Installed, or Error. The Installed panel uses the version from `InstallCompletion`, so it does not wait for another search. Uninstall calls `UninstallAppAsync`. That card shows Queued, Uninstalling, Not installed, or Error with Uninstall again. Update calls `UpdateAppAsync`. That card shows Queued, Downloading, Updating, Installed with the version from `UpdateCompletion`, or Error with Update again. A result that is not applicable stays Installed and keeps the current version. Progress and the final result are applied on the UI thread. A new search keeps that state for the same package id, including a version the catalog has not returned yet.

Beside search is the Source column. The combo has only Microsoft Store. Source and Sort by use `AnchoredComboBox`, so the list opens under the field. The selected row stays in its place in the list and does not cover the closed field.

Filters, on Discover only: All, Installed, and Not installed. Installed keeps Installed, Update available, and a card that is uninstalling, updating, or failed at either. Not installed keeps the rest, including an install that is still queued or downloading. Sort: Relevance (result order) and Name (`localeAwareCompare`). The search row and these three filters are hidden on Installed and Updates. Sort stays.

`QStackedWidget` has the list, Settings, About, and an application page added in code. The list is `QScrollArea#appScrollArea`. The horizontal scrollbar is off. The vertical scrollbar is 6 px, margin 0, with the thumb inset 1 px on each side. The list content bottom margin is 0 and the right margin is 2 px, so the cards stay wider.

A click on a card opens [[Application detail]] for that package. The search row and the filter row hide while that page is open. Back returns to the list and restores them for the current section. Choosing another sidebar section closes the page as well. Install, uninstall, update, and launch from the page use the same actions as the card. A status change for the open package refreshes the page. A failed launch shows its message on the page. If launch was started from the card, that failure opens the page and shows the message.

An empty list shows a label instead of the scroll area. Discover says nothing matched the search. Installed says there are no installed applications. Updates says there are no updates.

The Settings text says the catalog uses the Microsoft Store and that search matches the application name and package ID. The card grows with wrapped lines, so that sentence stays readable.

About shows the product name, version, a short description, Developed by Vanja Sretenovic, and a link to https://vanjasretenovic.com. The link opens in the browser.

## Filtering

Discover shows the current search. All, Installed, and Not installed filter that search. The Installed section calls `ListInstalledAppsAsync` and shows those Microsoft Store packages, without the three filters. Updates calls `ListUpdatesAsync` and shows packages with `IsUpdateAvailable`, plus a card that is updating or failed to update. Opening Settings or About does not clear the list. Coming back to the same section keeps it until another load starts.

Refreshing the list hides non-matches, sorts the visible cards, and restores the scroll position.

Related: [[Application card]], [[Application detail]], [[Mock catalog]], [[Decision 0003 Visual language]].
