---
title: Store search
tags:
  - core
  - search
status: current
updated: 2026-10-14
---

# Store search

`SearchApps` sends the query to the `msstore` catalog as the user typed it. The filter is `PackageMatchField::Name` with `ContainsCaseInsensitive`, and `ResultLimit` is 10. See [[WingetService]]. This project does not remove words before the call. The GUI shows only the matches the catalog returns.

## What the catalog does with common words

The catalog is a keyword index, not a scan of every package name character by character. Very common English words are dropped before the lookup. Words such as `what`, `the`, and `for` appear in a huge number of listings, so they do not tell one package from another. Keeping them would make the index larger and the search slower, and a query like `the app` would match almost everything that contains `the`.

The useful tokens are the ones that identify a product. `whats` in WhatsApp is one of those. `what` is not.

Observed on 2026-10-14:

| Query | Catalog result |
| --- | --- |
| `wha` | WhatsApp packages |
| `whats` | WhatsApp packages |
| `what` | No matches |

`what` is dropped. Nothing remains to look up, so the catalog returns an empty list. `wha` and `whats` are not dropped, so they still match WhatsApp. A local contains check on the name `WhatsApp` would still match `what`. The empty list is the catalog result, not a filter in ApplicationGUI.

The exact word list lives in the Store index. It is not in this repository, and this note does not treat that list as something we can read or change.

## Advantages we can aim for

These are product advantages over the Store index. None of them are implemented. `SearchApps` still forwards the typed query and displays the catalog answer.

- Match the characters the user typed. `what` should find WhatsApp, because that string is inside the name.
- Do not turn a short query into silence. If the only typed word is one the Store drops, say so, instead of an empty list with no reason.
- Search installed applications on the machine with a plain contains check. Those names are already local, so a common word does not have to erase them.
- Keep a product name whole when one piece of it is a common word, so a brand is not thrown away before the lookup.

A remote name search cannot invent hits the catalog refuses to return. Any of the advantages above needs a step beyond forwarding the query: a local installed-package scan, a second query strategy, or a clear empty-state message. The 10-result cap is separate. It is our `ResultLimit`, not the stop-word behavior.

Related: [[WingetService]], [[README outline]], [[Open questions]].
