# ED2K/eMule Reference Alignment Progress

This file is the compact chronological evidence trail for the ED2K/eMule
reference-alignment refactor.

Use this format:

```text
YYYY-MM-DD RAx status
Changed: concise tracker, code, or behavior summary.
Verified: exact final command and result, or documentation-only reason.
Remaining: next concrete gap.
Blocked: none, or exact blocker.
```

Keep entries checkpoint-sized. Do not record every investigation step, ordinary
build, small parser edit, raw public-network log, packet capture, generated
report, local cache, or conversation summary.

## Log

2026-05-19 RA0 verified
Changed: Replaced the previous active ED2K refactor tracker with a new
reference-alignment tracker. The new tracker separates the progress index,
reference ledger, domain checkpoint files, and compact progress log. It changes
the goal from public-network download completion to authoritative reference
alignment, protocol truth, maintainable ownership, and compact local
verification.
Verified: `git diff --check docs/maintenance` passed. CSV consistency checks
passed for `checkpoint-index.csv`, `reference-ledger.csv`, and every
`checkpoints/*.csv` file.
Remaining: Start RA1 current implementation ownership audit.
Blocked: none.

2026-05-19 RA1 verified
Changed: Audited the current ED2K implementation surface and recorded the
source inventory in `overview.md`. The audit maps focused helper modules,
RequestGroup-owned state, TCP command ownership, UDP/Kad ownership, sharing,
upload, session, RPC, documentation, and local test coverage. It also records
the immediate capability-truth risk around advertised eMule peer info and the
main maintainability risk in `Ed2kCommand.cc` and `Ed2kKadCommand.cc`.
Verified: Focused `rg` and `sed` inspection of ED2K source, integration, and
test files; `git diff --check docs/maintenance`; CSV consistency check for all
`docs/maintenance/ed2k-refactor` CSV files.
Remaining: Start RA2 authoritative reference mapping against amule-official,
emule-official-0.50a, mldonkey-official, wireshark-official, and protocol-docs.
Blocked: none.
