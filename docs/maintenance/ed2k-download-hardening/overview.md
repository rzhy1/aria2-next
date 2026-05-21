# ED2K Download Hardening

This tracker covers the next ED2K/eMule workstream after the completed
reference-alignment refactor. The prior tracker verified local protocol
surfaces. This tracker focuses on practical download reliability: long-running
source discovery, peer queue maintenance, LowID handling, retry policy, stable
identity, and transfer pacing.

## Tracker Files

| Path | Role |
| --- | --- |
| `overview.md` | Scope, reference policy, architecture rules, verification policy, and update rules |
| `roadmap.csv` | Short checkpoint index and current progress entry point |
| `capability-ledger.csv` | aMule-runtime capability decisions: port, adapt, replace, or prune |
| `progress.md` | Compact chronological evidence trail |
| `checkpoints/AR00-tracker.csv` | Tracker setup and baseline audit |
| `checkpoints/AR10-identity.csv` | Stable ED2K identity and persisted runtime metadata |
| `checkpoints/AR20-source-model.csv` | Source state, source limits, quality, and dead-source handling |
| `checkpoints/AR30-server-discovery.csv` | Server TCP/UDP discovery cadence and server source policy |
| `checkpoints/AR40-kad-discovery.csv` | Kad bootstrap, source search, publish, and maintenance cadence |
| `checkpoints/AR50-peer-lifecycle.csv` | Peer download state machine and scheduler ownership |
| `checkpoints/AR60-queue-reask.csv` | Queued-peer UDP reask and queue-rank refresh |
| `checkpoints/AR70-lowid-callback.csv` | LowID callback state, firewall boundaries, and fallback policy |
| `checkpoints/AR80-transfer-pacing.csv` | Block request pacing, timeout, cancel, retry, and range cleanup |
| `checkpoints/AR90-sharing-credits.csv` | Sharing, upload queue cooperation, credits, and peer fairness |
| `checkpoints/AR100-capability-docs.csv` | Capability truth, RPC/Motrix fields, documentation, and final verification |

Read `overview.md` and `roadmap.csv` first. Read only the active checkpoint
file during normal work. Read `capability-ledger.csv` when a port, adapt,
replace, or prune decision is being made or changed. Read the full tracker only
for final review or when a blocker requires cross-domain context.

## Reference Policy

The primary behavioral reference is:

| Reference | Role |
| --- | --- |
| `/Users/sekiro/Projects/oss/amule` | Practical ED2K/eMule runtime behavior for download queue processing, source maintenance, peer queue reask, Kad maintenance, LowID handling, sharing, upload, and credits |

Use the completed `docs/maintenance/ed2k-refactor` tracker as the current
aria2-next protocol baseline. Use local protocol references only when wire
format or capability truth must be rechecked. Do not copy reference code.

## Scope

The target is a more reliable native ED2K/eMule downloader inside aria2-next.
The work should keep the existing C++11 process, CMake build, Command/EventPoll
loop, DownloadEngine, RequestGroup, DownloadContext, PieceStorage, DiskAdaptor,
session, CLI, RPC, and documentation ownership.

The work should harden the current implementation around real P2P runtime
behavior:

| Area | Expected outcome |
| --- | --- |
| Identity | ED2K client identity is stable enough for queue, credit, and peer recognition behavior |
| Source model | Sources have explicit lifecycle state, quality, limits, retry, and dead-source expiry |
| Server discovery | TCP and UDP server source requests use bounded aMule-style cadence instead of noisy polling |
| Kad discovery | Kad bootstrap, source search, publish, refresh, and firewalled checks have durable scheduling |
| Peer lifecycle | Peer states are explicit enough to distinguish connecting, queued, downloading, no-needed-parts, callback waiting, dead, and retrying |
| Queue reask | Queued peers receive periodic `OP_REASKFILEPING`; `OP_REASKACK`, queue-full, and file-not-found responses update state |
| LowID | Callback states and unreachable-source boundaries are explicit and do not pollute normal direct-connection scheduling |
| Transfer pacing | Block request, timeout, cancel, range release, and retry behavior avoid stalls and busy loops |
| Sharing and credits | Cooperative upload behavior remains truthful and supports practical queue/credit interaction |
| Integration | RPC, Motrix Next fields, docs, and capability claims match the real supported behavior |

## AR00 Baseline Findings

The AR00 audit confirms that aMule's practical ED2K download behavior is driven
by long-running runtime loops, not only packet compatibility. The relevant
aMule entry points include `CDownloadQueue::Process`,
`CDownloadQueue::ProcessLocalRequests`, `CDownloadQueue::SendNextUDPPacket`,
`CDownloadQueue::DoKademliaFileRequest`, `CPartFile::Process`,
`CUpDownClient::IsSourceRequestAllowed`, `CUpDownClient::UDPReaskACK`,
`CUpDownClient::UDPReaskFNF`, `CUpDownClient::UDPReaskForDownload`,
`CDeadSourceList`, `CClientCredits`, `CUploadQueue`, `CSharedFileList`, and the
Kad search, publish, routing, and firewall paths.

The current aria2-next baseline already has native ED2K protocol modules,
server and Kad packet paths, peer handshake and transfer paths, sharing, upload
queue, credits, session serialization, RPC fields, and local tests. The runtime
hardening gap is in durable scheduling and state maintenance: identity
persistence, source quality, source expiry, server/Kad cadence, peer lifecycle,
queued-peer UDP reask, LowID callback boundaries, transfer pacing, and
integration status truth.

This tracker therefore starts from the completed `ed2k-refactor` protocol
baseline and does not reopen historical protocol-completion claims unless a
download-hardening checkpoint finds a concrete mismatch.

## Pruning Rules

All meaningful download-runtime capabilities start in scope. Prune only when
the behavior is obsolete, GUI-only, web-only, daemon-shell-only,
platform-specific, unsafe, privacy-sensitive, unmaintainable, or irrelevant to
aria2-next.

Expected prune or replace candidates include wx UI state, web UI, external
control shells, chat, friends, comments, preview, collections, GUI queue
controls, router helper stacks, and legacy database import paths. Do not prune
download-runtime behavior such as source lifecycle, queue reask, dead-source
handling, Kad discovery, LowID callback state, stable identity, upload queue
cooperation, or transfer pacing without specific evidence and user-visible
impact.

## Architecture Rules

Keep aria2-next ownership. Prefer focused native modules and explicit state
machines over a direct application-shell port from aMule. Refactor overloaded
ED2K files only when it improves correctness, ownership, or future maintenance.
Avoid mechanical line-count work.

Likely implementation owners:

| Area | Preferred aria2-next owner |
| --- | --- |
| Runtime identity and metadata | `Ed2kAttribute`, session serializer, control-file hidden state |
| Source lifecycle and scheduling | `ed2k_policy.*`, `PeerState`, request-level scheduling state |
| Server discovery cadence | server state in `Ed2kAttribute`, TCP path in `Ed2kCommand`, UDP path in `Ed2kKadCommand` |
| Kad cadence | `Ed2kKadCommand`, `Ed2kKadState`, `ed2k_kad.*`, `ed2k_kad_search.*` |
| Peer download lifecycle | `PeerState`, `ed2k_policy.*`, peer path in `Ed2kCommand` |
| Queue reask | UDP path in `Ed2kKadCommand` plus peer state in `Ed2kAttribute` |
| Transfer pacing | `Ed2kPeerTransfer`, `SegmentMan`, `PieceStorage`, `DiskAdaptor`, peer path in `Ed2kCommand` |
| Sharing and upload | `Ed2kSharedStore`, `Ed2kSharedResponder`, `Ed2kSharedPeerCommand`, `Ed2kUploadQueue` |
| RPC and docs | existing aria2-next RPC, manual, completion, and Motrix Next documentation paths |

## Verification Policy

Use compact local verification. Add tests only for state transitions, scheduler
timing, packet payload contracts, persistence, disk/integrity safety, RPC
fields, and confirmed regressions. Avoid public-network tests as automated
gates, socket-heavy fake integrations, broad scaffolding, placeholder tests,
and tests that only mirror implementation details.

Run focused tests only when a checkpoint needs evidence. Batch small edits when
safe. Final local verification requires:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

Public ED2K smoke testing is manual final evidence only. If public peers reset,
sources disappear, queue positions do not advance, or local LowID/firewall
state blocks transfer, record the exact operational boundary instead of
claiming deterministic public-network success.

## Update Rules

After each checkpoint, update `roadmap.csv`, the active checkpoint file,
`capability-ledger.csv` if a decision changes, and `progress.md` with compact
evidence. Do not commit raw logs, packet captures, generated reports, local
caches, public-network scratch artifacts, or conversation text.
