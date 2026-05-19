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

2026-05-19 RA2 verified
Changed: Completed the authoritative reference map. `overview.md` now records
the concrete reference entry points for aMule, eMule 0.50a, MLDonkey,
Wireshark, and protocol-docs. `reference-ledger.csv` remains the scope ledger
for port, adapt, replace, and prune decisions; domain rows stay pending until
their implementation checkpoints prove alignment.
Verified: Focused `find`, `rg`, and `sed` inspection of the authoritative
reference set; `git diff --check docs/maintenance`; CSV consistency check for
all `docs/maintenance/ed2k-refactor` CSV files.
Remaining: Start RA10 link and metadata alignment.
Blocked: none.

2026-05-19 RA10 verified
Changed: Aligned ED2K link and file identity parsing with the authoritative
aMule/eMule link behavior. File names now normalize path separators before
output path selection, file sizes reject values at or above the eMule/aMule
2^38 byte limit, empty part-hash lists are rejected, and `search` links parse
as metadata without becoming normal downloads. Updated the RA10 checkpoint and
reference ledger.
Verified: `cmake --build --preset default --target aria2_tests` passed.
`build/default/aria2_tests` passed with `OK (1093)`.
Remaining: Start RA11 metadata file and persistence alignment.
Blocked: none.

2026-05-19 RA11 verified
Changed: Aligned metadata file import and persistence ownership. `server.met`
parsing now preserves operational tags for users, files, limits, UDP flags,
LowID users, UDP key, obfuscation ports, and dynip hostnames into native
ServerState. `nodes.dat` parsing now filters unusable and Kad1-only contacts
and applies the aMule/eMule verified-contact fallback for bootstrap files.
Legacy part.met, known.met, known2.met, and clients.met remain replaced by
aria2-next control files, hidden session state, SharedStore, and UploadQueue
ownership.
Verified: `cmake --build --preset default --target aria2_tests` passed.
`build/default/aria2_tests` passed with `OK (1093)`.
Remaining: Start RA20 server TCP alignment.
Blocked: none.
