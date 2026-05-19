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

2026-05-19 RF1 partial
Changed: Reproduced the XP ED2K fixture in controlled 30 second runs under
`/Users/sekiro/Desktop/aria2-next-ed2k-debug`. Added packet-boundary debug
logging for ED2K command send and receive paths. Fixed three authoritative
reference mismatches found during root-cause investigation: standalone
`OP_EMULEINFO` now uses old MuleInfo tags instead of Hello tags and includes
the UDP port, comments, and compatible-client fields used by aMule/eMule;
outgoing Hello now uses the eMule hash marker, an aMule-compatible structured
version tag, conservative capability bits, and reference tag ordering; local
peer capabilities no longer advertise multipacket before the multipacket
request flow is implemented; and server `OP_FOUNDSOURCES_OBFU` source metadata
can be parsed and routed.
Verified: `PATH=/opt/homebrew/bin:$PATH cmake --build --preset default
--target aria2-next aria2_tests` passed. `git diff --check` passed for the
changed ED2K files. Short XP-link live runs after the changes still reached
only server discovery. Server `45.82.80.155:5687` returned ordinary
`OP_FOUNDSOURCES` with two sources. Both peers `152.67.253.33:8337` and
`175.0.74.131:2822` reset after local `OP_HELLO`; no peer packet was received
and the file remained 0 bytes. The last run directory was
`/Users/sekiro/Desktop/aria2-next-ed2k-debug/rf1-hello-version-20260519-113122`.
Remaining: Determine whether the current public XP sources are stale or
whether local outgoing peer hello still differs in a rejection-relevant way.
Next evidence should compare the exact sent hello bytes against aMule/eMule
and exercise a known local peer fixture that can answer hello.
Blocked: Public XP fixture currently provides only two reachable-looking
server sources, and both close before any peer response.
