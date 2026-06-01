# ED2K/eMule Reference Alignment Refactor

This directory is the active tracker for the ED2K/eMule reference-alignment
refactor in aria2-next. It replaces the previous `ed2k-refactor` tracker.

The earlier tracker mixed public-network download evidence, draft completion
claims, and implementation checkpoints. This tracker narrows the goal to
protocol alignment, maintainability, and local verification against the current
authoritative local references.

## Tracker Files

| Path | Role |
| --- | --- |
| `overview.md` | Scope, reference authority, architecture rules, verification policy, and update rules |
| `checkpoint-index.csv` | Main checkpoint index and current progress source |
| `reference-ledger.csv` | Reference subsystem decisions: port, adapt, replace, or prune |
| `progress.md` | Compact chronological evidence trail |
| `checkpoints/00-foundation.csv` | Tracker, baseline, and current-code audit checkpoints |
| `checkpoints/10-links-metadata.csv` | ED2K links, server.met, nodes.dat, file identity, and persisted metadata checkpoints |
| `checkpoints/20-server.csv` | ED2K server TCP/UDP, source discovery, HighID/LowID, callback, and server search checkpoints |
| `checkpoints/30-peer.csv` | Peer hello, capability truth, request flow, multipacket, file identifier, and transfer-control checkpoints |
| `checkpoints/40-transfer-integrity.csv` | Part transfer, compression, MD4, AICH, disk safety, and resume checkpoints |
| `checkpoints/50-discovery-policy.csv` | Source policy, Source Exchange, Kad, callback constraints, and retry checkpoints |
| `checkpoints/60-sharing-upload.csv` | Shared files, upload queue, credits, and cooperative peer behavior checkpoints |
| `checkpoints/70-search-rpc-docs.csv` | Search, RPC, Motrix Next fields, CLI/manual/completion docs, and final audit checkpoints |

Use `checkpoint-index.csv` as the single progress entry point. Use the
domain-specific checkpoint files for acceptance criteria and evidence. Use
`reference-ledger.csv` to prevent scope loss and to record every prune
decision.

## Reference Authority

The authoritative local reference set is:

| Reference | Role |
| --- | --- |
| `/Users/sekiro/Projects/oss/ed2k-references/amule-official` | Modern open eMule-compatible behavior for server, peer, Kad, sharing, upload, and search |
| `/Users/sekiro/Projects/oss/ed2k-references/emule-official-0.50a` | Canonical eMule packet flow, capability surfaces, file identifiers, secure identification, crypt, and UI-pruned behavior |
| `/Users/sekiro/Projects/oss/ed2k-references/mldonkey-official` | Independent implementation used to cross-check server, source, file, share, search, and Kad behavior |
| `/Users/sekiro/Projects/oss/ed2k-references/wireshark-official` | Packet framing, opcode, tag, and dissector cross-checks |
| `/Users/sekiro/Projects/oss/ed2k-references/protocol-docs` | Protocol text for ED2K, eMule extensions, Kad, links, corruption handling, HighID/LowID, and callback behavior |

Do not use deleted or disallowed references as implementation authority. Do not
copy reference code directly.

## Reference Map

RA2 maps the authoritative references to the tracker as follows:

| Reference | Protocol material used for alignment | Tracker ownership |
| --- | --- | --- |
| `amule-official` | `src/ED2KLink.*`, `Server.*`, `ServerList.*`, `ServerSocket.*`, `ServerUDPSocket.*`, `ClientTCPSocket.*`, `ClientUDPSocket.*`, `DownloadClient.*`, `DownloadQueue.*`, `KnownFile.*`, `PartFile.*`, `SharedFileList.*`, `UploadQueue.*`, `ClientCredits.*`, `SearchList.*`, `SearchExpr.h`, and `src/kademlia/` | Primary behavioral reference for links, server, peer, source policy, Kad, sharing, upload, credits, and search |
| `emule-official-0.50a` | `srchybrid/ED2KLink.*`, `ServerSocket.*`, `ClientUDPSocket.*`, `ListenSocket.*`, `DownloadClient.*`, `UploadClient.*`, `FileIdentifier.*`, `KnownFile.*`, `PartFile.*`, `SharedFileList.*`, `UploadQueue.*`, `ClientCredits.*`, `SearchList.*`, `SearchExpr.h`, `Encrypted*Socket.*`, and `srchybrid/kademlia/` | Canonical packet-flow and capability reference for peer identity, request flow, secure identification, obfuscation metadata, file identifiers, Kad, and UI-pruned behavior |
| `mldonkey-official` | `src/networks/donkey/donkeyProtoClient.ml`, `donkeyProtoServer.ml`, `donkeyProtoUdp.ml`, `donkeyProtoKademlia.ml`, `donkeyClient.ml`, `donkeyServers.ml`, `donkeyFiles.ml`, `donkeyShare.ml`, `donkeySearch.ml`, `donkeyUdp.ml`, and `donkeyNodesDat.mlp` | Independent cross-check for server and peer packets, Source Exchange, search, source policy, share state, UDP behavior, and nodes.dat handling |
| `wireshark-official` | `epan/dissectors/packet-edonkey.c` and `packet-edonkey.h` | Wire-format cross-check for packet framing, opcodes, tags, AICH, multipacket, compressed packets, source OBFU fields, server flags, and Kad packets |
| `protocol-docs` | `eMule-protocol_guide.txt`, `pDonkey-eDonkey-protocol-0.6.2.txt`, `aMule-Ed2k_link.html`, `aMule-Corruption_Handling.html`, `aMule-FAQ-eD2k-Kademlia.html`, `aMule-Kademlia.html`, `kademlia-paper-maymounkov-lncs.txt`, and `wireshark-edonkey-display-filter-reference.html` | Textual protocol reference for links, server and peer flow, HighID/LowID, callbacks, corruption handling, AICH, Kad concepts, file-size limits, and public-network operational boundaries |

The reference audit confirms that every meaningful ED2K/eMule subsystem is
represented in `reference-ledger.csv`. The rows are verified after the matching
implementation checkpoints prove alignment. RA2 proves that the reference
subsystems were mapped and that known non-core surfaces were either replaced or
pruned.

## Scope

The target is a practical native C++11 ED2K/eMule implementation aligned with
the authoritative references, integrated into aria2-next's existing CMake,
Command/EventPoll, DownloadEngine, RequestGroup, DownloadContext, PieceStorage,
DiskAdaptor, CLI, RPC, session, and documentation patterns.

The refactor must keep aria2-next as one native process. It must not introduce
Go, Java, sidecars, Boost, Asio, another daemon, another event loop, unrelated
architecture changes, or legacy GUI/daemon compatibility layers.

The implementation may improve on reference designs when aria2-next has a
safer or clearer native owner. Existing aria2-next disk, session, RPC, transfer,
and event-loop ownership should replace legacy client databases, GUI prompts,
daemon APIs, and platform-specific helper stacks.

## Final Refactor State

The codebase contains native ED2K modules for links, hashes, packet helpers,
server parsing, peer helpers, Kad state, source policy, AICH, compression,
task-level sharing, upload queue, request-group integration, session
serialization, RPC status, and documentation.

RA0 through RA71 are verified. The refactor aligns the native implementation
with the authoritative local references through local parser, packet, state,
integrity, persistence, RPC, and build verification. Public ED2K downloads stay
manual smoke evidence, not a checkpoint gate for this tracker.

Current source inventory after the RA1 audit:

| Area | Current files | Audit finding |
| --- | --- | --- |
| Link and file identity | `src/ed2k_link.*`, `src/ed2k_hash.*`, `src/ed2k_aich.*`, `src/ed2k_packet.*` | Focused parser, hash, AICH, and packet helpers exist and should remain stable unless reference comparison finds a wire-format bug |
| Request state | `src/Ed2kAttribute.*` | RequestGroup-owned coordinator for link metadata, server state, peer state, hashsets, AICH, search, Kad, and scheduling cursors |
| Server TCP and peer TCP | `src/Ed2kCommand.*` | Correct integration point for aria2-next TCP commands, but currently mixes server session, peer session, packet dispatch, request flow, AICH, Source Exchange, transfer, and partial upload response behavior |
| Server UDP and Kad | `src/Ed2kKadCommand.*`, `src/Ed2kKadState.*`, `src/ed2k_kad.*`, `src/ed2k_kad_search.*` | Correct single UDP event-loop owner, but ED2K server UDP and Kad traversal are interleaved in one command |
| Source policy | `src/ed2k_policy.*` | Focused source selection and piece-selection helpers preserve retry, queue, availability, and endgame behavior |
| Transfer and disk safety | `src/Ed2kPeerTransfer.*`, `src/ed2k_compression.*` | Existing disk path is used through `PieceStorage` and `DiskAdaptor`; RA40 verified range and corruption paths |
| Sharing and upload | `src/Ed2kShareIndex.*`, `src/Ed2kSharedFile.*`, `src/Ed2kSharedResponder.*`, `src/Ed2kUploadQueue.*` | Active ED2K request groups expose verified pieces and completed files as task-level shared sources; upload queue, responder, and credit surfaces align with non-pruned reference behavior |
| CLI/RPC/session | `src/download_helper.cc`, `src/SessionSerializer.cc`, `src/RpcMethodImpl.cc`, `src/OptionHandlerFactory.cc`, `docs/completion/aria2-next` | Existing aria2-next integration points are preserved; RA70 and RA71 verified field and documentation truth |
| Tests | `tests/Ed2kHelperTest.cc`, `tests/Ed2kKadStateTest.cc`, `tests/Ed2kShareIndexTest.cc`, `tests/Ed2kUploadQueueTest.cc`, `tests/ProtocolDetectorTest.cc` | Coverage is local and parser/state focused; `tests/Ed2kHelperTest.cc` is broad and should be split only when future changes need clearer ownership |

Capability truth was aligned in RA30. Local peer info advertises implemented
AICH, Unicode, compression, Source Exchange 1, extended requests, large files,
and Source Exchange 2. Multipacket, extended multipacket, secure
identification, crypt, Kad peer capability, and file identifier envelopes stay
unadvertised until complete packet and state ownership exists.

The largest current maintainability risk is overloaded command ownership:
`src/Ed2kCommand.cc` still mixes server session work, peer download session
work, packet dispatch, request sequencing, Source Exchange, AICH, upload
responses, and transfer handling. `src/Ed2kKadCommand.cc` mixes Kad traversal
and ED2K server UDP behavior. Split only when it improves correctness,
capability truth, or future maintenance.

## Architecture Rules

Keep the implementation native and local to aria2-next. Prefer focused modules
with explicit state ownership:

| Area | Preferred owner |
| --- | --- |
| Link and metadata | `ed2k_link.*`, `Ed2kAttribute`, `DownloadContext` |
| Packet framing and tags | `ed2k_packet.*` and narrow protocol helper modules |
| Server TCP | A focused server-session path for login, IDChange, source requests, callback, status, search, server list, retry, and persistence |
| Server UDP | Existing UDP command path with clear ED2K server UDP ownership and bounded packet parsing |
| Peer download | A focused peer-download session path for hello, capability truth, request flow, queue state, parts, compression, AICH, and failure handling |
| Peer upload and sharing | `Ed2kShareIndex`, `Ed2kSharedResponder`, `Ed2kSharedFile`, and `Ed2kUploadQueue` |
| Source policy | `ed2k_policy.*`, `PeerState`, and request-level scheduling state |
| Kad | `Ed2kKadCommand`, `Ed2kKadState`, and focused Kad packet/traversal helpers |
| Persistence | Existing aria2-next session/control-file mechanisms plus documented hidden ED2K state |
| CLI/RPC/docs | Existing aria2-next option, status, RPC, manual, completion, and Motrix Next documentation paths |

Refactor overloaded files by ownership, not by mechanical line-count churn.
Delete duplicated or wrong draft code when the reference audit shows it is the
wrong shape.

## Verification Policy

Use compact local verification. Add tests only for parser contracts, packet
framing, state transitions, persistence formats, disk and integrity safety, RPC
field contracts, and confirmed regressions.

Avoid socket-heavy fake integration tests, broad scaffolding, placeholder
tests, public-network tests as automated gates, tests for incidental logging,
and tests that simply mirror implementation details.

Run intermediate verification only when it closes a checkpoint or protects a
risky protocol boundary. Batch ordinary parser edits, small cleanup, and module
splits. Final local verification requires:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

Public ED2K smoke testing is manual final evidence. If public peers reset,
sources disappear, or LowID/firewall state blocks transfer, record the boundary
as operational evidence rather than blocking reference-alignment completion.

Final RA71 local verification passed with `cmake --preset default`,
`cmake --build --preset default`, `ctest --preset default`, and
`build/default/aria2-next --version`. Completion generation and shell syntax
checks for `docs/completion/aria2-next` also passed against the current binary.

## Update Rules

After each checkpoint, update `checkpoint-index.csv`, the matching
`checkpoints/*.csv` row, `reference-ledger.csv` when a reference decision
changes, and `progress.md` with one compact entry.

Keep maintenance records durable and compact. Do not commit raw logs, packet
captures, generated reports, caches, temporary API payloads, local public
network fixtures, or conversation text.
