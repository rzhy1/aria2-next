# Libtorrent BitTorrent Replacement

This tracker owns the full replacement of aria2-next's native BitTorrent
engine with libtorrent-rasterbar. The replacement is intentionally not a
backward-compatible dual-backend migration. Once the libtorrent path is
working, obsolete native BitTorrent runtime code, options, docs, completion
entries, and tests must be removed.

## Tracker Files

| Path | Role |
| --- | --- |
| `overview.md` | Scope, architecture rules, dependency policy, cleanup policy, verification policy, and update rules |
| `roadmap.csv` | Single checkpoint index and current progress entry point |
| `capability-ledger.csv` | Capability-by-capability migration, sharing, and removal decisions |
| `progress.md` | Compact chronological evidence trail |
| `checkpoints/BTM-001-foundation.csv` | Tracker activation, dependency facts, source inventory, and final goal contract |
| `checkpoints/BTM-002-build.csv` | libtorrent dependency detection, compile mode, CMake integration, and release dependency boundaries |
| `checkpoints/BTM-003-ingress.csv` | Torrent file and magnet request creation routed into the libtorrent backend |
| `checkpoints/BTM-004-runtime.csv` | libtorrent session ownership, alert polling, torrent lifecycle, and event-loop integration |
| `checkpoints/BTM-005-status-progress.csv` | RPC, console, completion state, verified progress, and 99 percent BitTorrent tail-stall semantics |
| `checkpoints/BTM-006-persistence.csv` | Resume data, metadata save/load, session serialization, and restart behavior |
| `checkpoints/BTM-007-controls.csv` | Pause, resume, remove, file selection, speed limits, seed limits, and error mapping |
| `checkpoints/BTM-008-native-cleanup.csv` | Native BitTorrent source, stale option, stale doc, and stale test removal |
| `checkpoints/BTM-009-non-bt-stall.csv` | HTTP, FTP, and SFTP zero-speed or 99 percent stall watchdog separated from BitTorrent |
| `checkpoints/BTM-010-docs-packaging.csv` | Manual, completion, packaging, dependency, and release-path updates |
| `checkpoints/BTM-011-validation.csv` | Final build, test, smoke, and comparison validation |

Read `overview.md` and `roadmap.csv` first. During implementation, read only
the active checkpoint and `capability-ledger.csv`. Read the full tracker only
for final review or when a blocker crosses checkpoint boundaries.

## Goal Contract

Replace aria2-next's native BitTorrent engine with libtorrent-rasterbar as the
only torrent and magnet implementation. Keep HTTP, FTP, SFTP, ED2K, Metalink,
RPC, CLI, session, and release behavior intact unless a change is required to
connect libtorrent cleanly. Remove the obsolete native BitTorrent scheduler,
peer protocol, DHT, tracker, PEX, LPD, piece picker, endgame, metadata, resume,
and stale test surfaces after the libtorrent path owns them. Fix the
BitTorrent 99 percent tail-stall and false-completion behavior through
libtorrent-backed verified progress, completion, and resume state. Fix the
separate non-BitTorrent 0B or 99 percent stall path with a focused native
watchdog. Stop only when local build, test, version, documentation, and
sequential public BitTorrent smoke evidence pass.

## Reference Policy

The primary external implementation reference is libtorrent-rasterbar. It owns
BitTorrent peer protocol, DHT, tracker, PEX, LSD, uTP, piece picking, endgame,
disk I/O, fast resume, torrent metadata, and BitTorrent v2 behavior.

Use qBittorrent and Free Download Manager only as practical ecosystem evidence
that libtorrent-rasterbar is a mature production backend. Do not copy code from
those clients. Use upstream aria2 only to identify old behavior that must be
preserved outside BitTorrent or removed inside BitTorrent.

## Scope

All torrent and magnet downloads must use libtorrent-rasterbar. The native
BitTorrent backend must not remain as a fallback, compatibility mode, hidden
option, or release-time alternate path.

Shared aria2-next infrastructure remains in scope when it is still useful:
`RequestGroup`, `DownloadEngine`, `Option`, RPC methods, session save/load,
console status, global speed limits, logging, URI ingestion, and release
packaging. Native BitTorrent-only modules are removal candidates once their
responsibility has moved to libtorrent.

The C++ baseline may diverge by build path if required by libtorrent. The
tracker must record whether the whole project moves beyond C++11 or only the
libtorrent-enabled files require a newer standard.

## 99 Percent Policy

BitTorrent progress must not rely on in-flight piece bytes as completed bytes.
User-facing progress may show received data, but completion, RPC final state,
control-file cleanup, and resume decisions must use verified libtorrent state.

The non-BitTorrent 0B or 99 percent stall problem is separate. Its watchdog
must live in the native HTTP, FTP, and SFTP command path and must not be hidden
inside the libtorrent migration.

## Test Policy

Tests must be few and high value. Add tests for routing, state mapping,
completion truth, resume behavior, file selection, option removal, and the
non-BitTorrent stall watchdog. Do not add tests that only mirror libtorrent
internals, incidental logs, or removed native BitTorrent implementation
details.

Delete obsolete native BitTorrent tests when their code path is removed. Keep
parser or metadata tests only if they still protect non-BitTorrent behavior or
the new libtorrent ingestion boundary.

## Cleanup Policy

Remove native BitTorrent code after each migrated owner is proven through the
active checkpoint. Remove stale CMake source entries, preprocessor guards,
option handlers, usage text, manual sections, completion entries, and tests in
the same checkpoint that removes the behavior.

Do not leave dead compatibility aliases unless an existing public CLI option
must fail with a clear replacement message for one release.

## Verification Policy

Use focused local verification during checkpoints. Final verification requires:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

Public BitTorrent smoke comparisons are manual evidence, not deterministic
unit-test gates. Store temporary downloads, logs, and captures under
`/Users/sekiro/Desktop/aria2-next-current`. Do not commit public-network logs,
packet captures, generated reports, or local caches.

## Update Rules

After each checkpoint, update `roadmap.csv`, the matching checkpoint file,
`capability-ledger.csv` when a decision changes, and `progress.md` with compact
evidence. Keep entries checkpoint-sized and durable.
