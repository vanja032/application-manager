---
title: README outline
tags:
  - readme
status: current
updated: 2026-10-14
---

# README outline

This is not the README. It is the contents the root README should cover once the vault is dense enough. Each item already has a source note.

The assessment deliverable needs these sections first. Source: [[Production goal]], [[Decision 0006 Stack]], [[Build]].

- Prerequisites and dependencies.
- Build instructions.
- A short architectural overview of the winget integration and the UI framework.

Then:

1. What the application does and who it is for. Source: [[Application overview]].
2. What the solution contains and what each project is allowed to know. Source: [[Solution]], [[Decision 0001 UI before backend]].
3. Layout of `include/`, `src/`, `ui/`, and `resources/`. Source: [[Folder layout]].
4. How to build Debug x64 and where the executable lands. Source: [[Build]].
5. What the backend can do: search, install, update, uninstall, `msstore` catalog. Source: [[WingetService]].
6. What the GUI shows today and what is still mock data. Source: [[ApplicationGUI]], [[Mock catalog]].
7. Visual rules: square corners, light and dark theme. Source: [[Decision 0003 Visual language]].
8. Known limits. Source: [[Open questions]].

Write the README only when these items are no longer just an outline. The language is English.
