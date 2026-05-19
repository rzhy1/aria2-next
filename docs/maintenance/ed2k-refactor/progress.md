# Native ED2K/eMule Refactor Progress

This file is the compact chronological evidence trail for the ED2K/eMule
refactor. Append one entry after every checkpoint, root-cause finding, or
material cleanup.

Use this format:

```text
YYYY-MM-DD RFx status
Changed: concise file and behavior summary.
Verified: exact command and result, or documentation-only reason.
Remaining: next concrete gap.
Blocked: none, or exact blocker.
```

## Log

2026-05-19 RF0 partial
Changed: Created the active ED2K refactor tracker. The tracker records the
authoritative reference set, the known live interoperability failure, the
required root-cause-first workflow, the checkpoint matrix, and the reference
audit ledger.
Verified: Documentation-only tracker creation in progress.
Remaining: Update the maintenance index and mark the previous ED2K tracker as
historical.
Blocked: none.

2026-05-19 RF0 verified
Changed: Updated the maintenance index so `ed2k-refactor` is the active ED2K
tracker, and marked the previous `ed2k` tracker as historical.
Verified: Documentation-only tracker activation. `git diff --check
docs/maintenance` passed. CSV width checks for `checkpoints.csv` and
`reference-audit.csv` passed after removing trailing blank records.
Remaining: Start RF1 with a short live-failure baseline before changing ED2K
source code.
Blocked: none.
