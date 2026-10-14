---
title: How we document
tags:
  - process
status: current
updated: 2026-10-14
---

# How we document

Notes are the working memory of the project. After each crucial task, update the existing note. Do not start a new README.

## When the vault changes

A crucial task is a new feature, an architecture change, a project layout change, a build setup change, or a decision that later documentation depends on. A small visual correction goes into the existing component note, without a new decision.

## Where the record goes

1. Update the note for the project or component the task touched.
2. If the task records a choice, add a note under `01 Decisions/` and link it from [[Decision index]].
3. Add one dated line to [[Log]].
4. If the task opens or closes a question, edit [[Open questions]].
5. If the future README content changes, update [[README outline]].
6. If the task moves a requirement in [[Production goal]], change that row's status. A row is Met only when the GUI does it against the live Microsoft Store catalog without blocking.

The repository root stays without a README until the final text is derived from the vault.

## Note shape

- Language is English. Class names, paths, and APIs stay as they are in the code.
- The file name is unique across the vault, because Obsidian links resolve by note name.
- Links use the form `[[Note name]]`.
- The top of each note has short frontmatter: `title`, `tags`, `status`, `updated`.
- `status` is `current` while the description holds, or `superseded` when a newer decision replaces it.
- A note describes behavior and points at the source path. It does not copy whole source files.
