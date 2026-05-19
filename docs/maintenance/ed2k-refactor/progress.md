# Native ED2K/eMule Refactor Progress

This file is the compact evidence trail for the ED2K/eMule refactor. Keep it
checkpoint-sized. Do not record every investigation step, every routine local
build, or every small test run.

Verification should be batched. Run intermediate tests only when they prove a
root cause, protect a risky protocol boundary, or close a checkpoint. Do not
append test-only notes unless the result changes checkpoint state or records a
durable blocker.

Use this format:

```text
YYYY-MM-DD RFx status
Changed: concise file and behavior summary.
Verified: final command and result for the completed batch, or
documentation-only reason.
Remaining: next concrete gap.
Blocked: none, or exact blocker.
```

## Log

2026-05-19 RF0 verified
Changed: Updated the maintenance index so `ed2k-refactor` is the active ED2K
tracker, marked the previous `ed2k` tracker as historical, and created the
refactor overview, checkpoint matrix, reference audit, and progress log.
Verified: Documentation-only tracker activation. `git diff --check
docs/maintenance` passed. CSV width checks for `checkpoints.csv` and
`reference-audit.csv` passed after removing trailing blank records.
Remaining: none for RF0.
Blocked: none.

2026-05-19 RF1 verified
Changed: Closed the live failure baseline checkpoint using the existing
controlled public runs under `/Users/sekiro/Desktop/aria2-next-ed2k-debug`.
The baseline records the XP fixture and Windows 11 fixture behavior without
claiming public transfer success. XP runs consistently parsed the link,
created the task, connected to ED2K servers, received LowID warnings, received
one or two server sources, sent local peer `OP_HELLO`, then saw the peer reset
before any peer packet. A Windows 11 x64 run reached `OP_HELLOANSWER`, sent
`OP_EMULEINFO` and `OP_REQUESTFILENAME`, received `OP_EMULEINFOANSWER`, and
closed before a file answer; later runs against the same public source reset
after local `OP_HELLO`.
Verified: Documentation-only checkpoint closure based on existing controlled
logs:
`/Users/sekiro/Desktop/aria2-next-ed2k-debug/rf1-hello-version-20260519-113122/aria2-ed2k-test.log`,
`/Users/sekiro/Desktop/aria2-next-ed2k-debug/rf1-win11-x64-20260519-113528/aria2-ed2k-test.log`,
and
`/Users/sekiro/Desktop/aria2-next-ed2k-debug/rf1-win11-x64-extreq-20260519-114314/aria2-ed2k-extreq.log`.
The packet boundary is server source discovery succeeded, public peer progress
is nondeterministic under LowID, and RF3 must classify source and callback
paths before direct peer failures can be treated as pure peer-handshake bugs.
Remaining: Start RF3 by auditing and correcting server source classification,
HighID/LowID state, OBFU metadata, and callback routing.
Blocked: none.

2026-05-19 RF2 verified
Changed: Completed the authoritative reference audit using only
`amule-official`, `emule-official-0.50a`, `mldonkey-official`,
`wireshark-official`, and `protocol-docs`. The audit covers links, metadata
files, server TCP/UDP, OBFU, HighID/LowID, callback, peer handshake,
capabilities, request flow, Source Exchange, AICH, compressed transfer, Kad,
search, scheduling, resume, sharing, upload, credits, CLI/RPC/Motrix fields,
persistence, and prune-only legacy surfaces.
Verified: Documentation-only checkpoint closure. Coverage check, CSV width
checks, and `git diff --check docs/maintenance/ed2k-refactor` passed.
Remaining: none for RF2.
Blocked: none.

2026-05-19 RF3/RF5 partial
Changed: Advanced server-source compatibility and preserved adjacent
request-flow fixes found during baseline work. Server IDChange and status now
retain obfuscation and extended UDP metadata. TCP and UDP source parsing
preserves LowID classification, client ID, user hash, and crypt metadata.
Large-file source requests are gated by server capability. TCP source requests
use `OP_GETSOURCES_OBFU` when the server advertises TCP source obfuscation.
Extended callback-requested payloads preserve endpoint, crypt options, and
user hash fields. Peers requiring encrypted transport are not scheduled into
the current plaintext path. Peer hello parsing reads eMule misc option tags.
Extended filename requests include part status and complete-source count when
advertised. File-status bitfields use reference wire order, and single-part
files skip the extra request-file-id step after filename answer.
Verified: One focused local batch verification passed for the affected build
and loopback protocol surface. Future work should batch related RF3/RF5 changes
and run one focused verification after the batch, with live ED2K checks reserved
for checkpoint closure or final interoperability evidence.
Remaining: RF3 still needs callback-fail state handling, UDP source/status
closure, and final server-source evidence. RF5 still needs multipart
status/hashset sequencing, multipacket variants, file identifiers, controlled
queue/transfer state verification, and later live evidence.
Blocked: none.
