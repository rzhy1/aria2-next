# Maintenance

This directory contains durable project maintenance records. These files are
useful for long-running stewardship, but they are not part of the build, test,
or release execution path.

The root of this directory intentionally contains only this index. Each
maintenance task owns its own subdirectory.

| Path | Purpose |
| --- | --- |
| `cmake-migration/progress.md` | Completed checkpoint log for the CMake migration |
| `upstream-issue-review/summary.md` | Completed human-readable summary of the upstream issue review |
| `upstream-issue-review/matrix.csv` | Completed issue-by-issue review matrix |
| `ed2k/overview.md` | Historical tracker for the first native ED2K/eMule draft |
| `ed2k/checkpoints.csv` | Historical checkpoint matrix for the first ED2K/eMule draft |
| `ed2k/reference-parity.csv` | Historical parity ledger for the first ED2K/eMule draft |
| `ed2k/progress.md` | Historical chronological ED2K/eMule draft log |
| `ed2k-refactor/overview.md` | Active tracker entry point for the ED2K/eMule interoperability refactor |
| `ed2k-refactor/checkpoints.csv` | Active checkpoint matrix for the ED2K/eMule refactor |
| `ed2k-refactor/reference-audit.csv` | Active authoritative reference audit and pruning ledger |
| `ed2k-refactor/progress.md` | Active chronological ED2K/eMule refactor progress log |

Maintenance records should be durable and compact. Temporary API payloads,
scratch research files, generated reports, local caches, and conversation logs
should not be committed here.

Release history is tracked by git tags and GitHub Releases. Do not add
standalone changelog snapshots unless they are generated release artifacts.
