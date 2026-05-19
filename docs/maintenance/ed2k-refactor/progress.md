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

2026-05-19 RF3/RF4/RF5 partial
Changed: Advanced the server-source compatibility path without claiming RF3
completion. Server IDChange parsing now preserves the TCP obfuscation port
advertised by modern servers. Server status updates now keep the extended
status fields used for UDP and obfuscation decisions. Server and UDP
FoundSources handling now uses the richer source model so LowID sources and
crypt-required sources are not treated as ordinary direct TCP peers. Packed UDP
FoundSources parsing now keeps client ID and LowID classification. Large-file
TCP GetSources requests are gated by the connected server's advertised large
file capability, matching the aMule/eMule behavior that skips unsupported
large-file source requests instead of sending a request older servers cannot
handle. Peer hello parsing now reads eMule misc option tags from Hello and
HelloAnswer packets, which supports the RF4 capability truth work already in
progress. ED2K command-level tests were then narrowed to bounded loopback
protocol checks so the full `aria2_tests` entry point no longer performs
public ED2K network work or waits on real remote peers.
Verified: `cmake --preset default` passed after the build directory was
recreated. `cmake --build --preset default --target aria2_tests -j 1` passed.
`cmake --build --preset default --target aria2-next -j 1` passed with the
existing non-fatal local linker warning about `/opt/homebrew/opt/tcl-tk/lib`.
`build/default/aria2_tests` passed with `OK (1114)` after the test cleanup, and
`build/default/aria2_tests 2>&1 | awk '/Connecting to ED2K/ { print; if ($0 !~
/127[.]0[.]0[.]1/) bad = 1 } END { exit bad ? 1 : 0 }'` printed only loopback
ED2K peer/server connections and exited 0. `git diff --check` passed.
Remaining: RF3 still needs ordinary and OBFU GetSources selection to be fully
audited, callback requested/fail routing to be completed, UDP global sources
and status behavior to be closed, and server-state persistence to be verified
before the checkpoint can become verified.
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

2026-05-19 RF1/RF5 partial
Changed: Tested the newer Windows 11 x64 ED2K fixture in controlled 30 second
runs under `/Users/sekiro/Desktop/aria2-next-ed2k-debug`. The fixture is
better than the XP link for request-flow diagnosis because public servers
returned a source for hash `f3c8ae69aa86f1fc8204cc9bc4d9f61b`. A pre-fix run
reached peer `58.163.147.2:4377`, received `OP_HELLOANSWER`, sent
`OP_EMULEINFO` plus `OP_REQUESTFILENAME`, received `OP_EMULEINFOANSWER`, and
then saw the peer close before any file answer. Authoritative aMule/eMule
references show that when `extendedRequestsVersion > 0`, `OP_REQUESTFILENAME`
must include part-status data after the file hash, and when the version is
greater than 1 it must also include complete-source count. The draft
advertised extended request version 2 but sent only the 16 byte file hash.
Implemented an aria2-next native request payload builder that writes hash,
ED2K part count, local completed-part bitfield, and complete-source count.
Incoming shared-peer file requests now accept modern extended payloads and
pass the hash portion to the shared responder. A compact regression assertion
was added to the existing peer hello test to verify that the first outgoing
`OP_REQUESTFILENAME` is no longer the 16 byte short packet that aMule treats
as invalid after advertising extended requests.
Verified: `PATH=/opt/homebrew/bin:$PATH cmake --build --preset default
--target aria2-next aria2_tests` passed. A post-fix 30 second public run with
the same Windows 11 x64 fixture still ended at 0 bytes, but that run did not
reach the fixed request-flow boundary: server `45.82.80.155:5687` returned
one source, then peer `58.163.147.2:4377` reset immediately after local
`OP_HELLO` with no peer packet received. All observed servers assigned LowID
and emitted the public warning that the local network configuration is not
externally reachable. The post-fix run directory was
`/Users/sekiro/Desktop/aria2-next-ed2k-debug/rf1-win11-x64-extreq-20260519-114314`.
Remaining: Continue RF1/RF5 with controlled local peer fixtures and later
public runs that reach `OP_REQFILENAMEANSWER`, `OP_FILESTATUS`, queue, or part
request state. Public single-source runs cannot prove the request-flow fix
when the peer closes before answering hello.
Blocked: Public live verification is nondeterministic while the client is
LowID and current fixtures provide only one or two reachable-looking sources.

2026-05-19 RF5 partial
Changed: Corrected ED2K file-status bitfield packing and parsing to use the
least-significant-bit-first order used by aMule and eMule `WritePartStatus`
and `ProcessExtendedInfo`. The previous local helper round-tripped its own
high-bit-first encoding, but that encoding would invert peer part availability
within each byte when talking to real clients. Added one byte-level assertion
to the existing protocol payload test so the wire order is pinned to the
reference behavior without adding new scaffolding.
Verified: `PATH=/opt/homebrew/bin:$PATH cmake --build --preset default
--target aria2-next aria2_tests` passed.
Remaining: Continue RF5 request-flow work with file status, hashset, queue,
start-upload, accept-upload, and part request sequencing under controlled peer
fixtures before relying on public network runs.
Blocked: none.

2026-05-19 RF5 partial
Changed: Aligned the plain non-multipacket post-filename path with aMule and
eMule behavior for single-part files. After `OP_REQFILENAMEANSWER`, aria2-next
now skips `OP_SETREQFILEID` when the file is no larger than one ED2K part and
continues to source exchange plus `OP_STARTUPLOADREQ`. This avoids sending the
file-status request that the reference clients deliberately removed for
single-part files for better eDonkeyHybrid compatibility. The existing
outgoing peer test now asserts that a single-part request does not emit
`OP_SETREQFILEID` after a valid filename answer.
Verified: `PATH=/opt/homebrew/bin:$PATH cmake --build --preset default
--target aria2-next aria2_tests` passed.
Remaining: Continue RF5 for multi-part status/hashset sequencing,
multi-packet variants, file identifiers, and controlled queue/transfer state
verification.
Blocked: none.

2026-05-19 RF2 partial
Changed: Split the RF2 metadata audit row into focused durable rows for file
links, server and nodes links, server.met parsing, and nodes.dat parsing. The
new rows are based on the current authoritative local references: aMule
`ED2KLink`, eMule `ED2KLink`, the local aMule ED2K link protocol document,
aMule/eMule server list and server metadata loaders, MLDonkey server.met
import paths, and aMule/eMule/MLDonkey Kad contact loading paths. The audit now
separates behavior to adapt from legacy GUI update prompts and local UI
history that should be pruned or replaced by aria2-next task/session plumbing.
Verified: Documentation-only audit refinement. CSV width check for
`checkpoints.csv` and `reference-audit.csv` passed.
Remaining: Continue RF2 by auditing search-link/UI-only surfaces, social
packet surfaces, known-file metadata, and persistence formats before marking
RF2 verified.
Blocked: none.

2026-05-19 RF2 partial
Changed: Expanded the active authoritative reference audit for the remaining
metadata and integration surfaces. Search links, search filters and result
metadata, known/shared file metadata, part.met replacement, AICH persistence,
peer credits and secure identity, upload queue policy, obsolete social packet
surfaces, and shared-directory browsing are now tracked as separate decisions
instead of being hidden inside broad search, persistence, sharing, or prune
rows.
Verified: Documentation-only audit refinement based on local authoritative
references under `amule-official`, `emule-official-0.50a`,
`mldonkey-official`, `wireshark-official`, and `protocol-docs`.
Remaining: Continue RF2 by auditing packet-level RF3-RF5 capability rows in
the same detail before marking the reference audit verified.
Blocked: none.

2026-05-19 RF2 partial
Changed: Expanded the packet-level authoritative reference audit for the live
interoperability risk areas. Server IDChange and capability flags, TCP and UDP
source request variants, FoundSources OBFU parsing, callback routing, LowID
impossible paths, peer capability truth, large-file/Unicode/compression
claims, modern file request sequencing, multipacket envelopes, file
identifiers, and transfer failure packets now have separate durable rows.
Verified: Documentation-only audit refinement based on local authoritative
references under `amule-official`, `emule-official-0.50a`,
`mldonkey-official`, `wireshark-official`, and `protocol-docs`.
Remaining: Validate the tracker shape, then continue RF2 by checking whether
any remaining reference subsystem is still hidden inside broad rows before
marking RF2 verified.
Blocked: none.

2026-05-19 RF2 partial
Changed: Completed another reference-audit pass over the remaining broad
surfaces. Server shared-file publishing, file status and hashset tolerance,
client UDP reask and queue state, server and UDP search wire formats,
remote-control APIs, and NAT auto-mapping helpers now have separate decisions
instead of being folded into sharing, request flow, search, GUI, or networking
rows.
Verified: Documentation-only audit refinement based on focused inspection of
local authoritative references and the active tracker. No build was required
because no source or generated files changed.
Remaining: Run tracker validation, then decide whether RF2 can be marked
verified or whether any final subsystem remains unclassified.
Blocked: none.

2026-05-19 RF2 verified
Changed: Marked the authoritative reference audit checkpoint verified. The
active audit now classifies all goal-scope ED2K/eMule subsystems across links,
server TCP/UDP, metadata files, OBFU, HighID/LowID, callback, peer handshake,
capability bits, multipacket, file identifiers, secure identification,
crypt/obfuscation, file requests, hashsets, Source Exchange, AICH, compressed
transfer, Kad, search, scheduling, resume, sharing, upload queue, credits,
CLI/RPC/Motrix fields, persistence, and prune/replace-only legacy surfaces.
Verified: Documentation-only checkpoint closure. Coverage keyword check
against the goal scope found no missing top-level subsystem. Tracker CSV width
checks and `git diff --check docs/maintenance/ed2k-refactor` passed.
Remaining: Start RF3 server source compatibility.
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

2026-05-19 RF3 partial
Changed: Aligned two server-source paths with the authoritative aMule/eMule
behavior without running a live ED2K transfer. TCP source requests now use
`OP_GETSOURCES_OBFU` when the connected server advertises TCP source
obfuscation support, so compatible servers can return OBFU source metadata
instead of plain endpoint-only sources. Callback-requested parsing now accepts
extended payloads with trailing data while preserving the endpoint, crypt
options, and user hash fields used by modern clients. Callback-returned peers
that require encrypted transport are not scheduled into the current plaintext
peer connection path.
Verified: `cmake --build --preset default --target aria2_tests -j 1` passed
with the existing local linker warning about `/opt/homebrew/opt/tcl-tk/lib`.
`build/default/aria2_tests` passed with `OK (1114)`.
Remaining: RF3 still needs callback-fail state handling, UDP source/status
closure, and final server-source live evidence before the checkpoint can be
marked verified.
Blocked: none.
